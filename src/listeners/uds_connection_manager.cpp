#include "uds_connection_manager.hpp"
#include "../media/media_manager.hpp"
#include "../mpi_ctx/mpi_ctx_intf.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace {

const int MEDIA_TYPE_READ_TIMEOUT_MS = 5000;
const int CLEANUP_POLL_INTERVAL_MS = 1000;

} // namespace

uds_connection_manager::uds_connection_manager(media_manager *mgr, const std::string &uds_path)
    : mgr_(mgr)
    , uds_path_(uds_path)
    , listen_fd_(-1)
    , running_(false)
{
}

uds_connection_manager::~uds_connection_manager()
{
    stop();
}

bool uds_connection_manager::read_media_type(int fd, int timeout_ms, media_type *out_type)
{
    if (!out_type)
        return false;

    struct pollfd pfd = { fd, POLLIN, 0 };
    int r = poll(&pfd, 1, timeout_ms);
    if (r <= 0)
        return false;

    unsigned char t = 0xff;
    ssize_t n = recv(fd, &t, 1, 0);
    if (n != 1)
        return false;

    if (t == 0 || t == 'v' || t == 'V') {
        *out_type = media_type::video;
        return true;
    }
    if (t == 1 || t == 'a' || t == 'A') {
        *out_type = media_type::audio;
        return true;
    }
    return false;
}

int uds_connection_manager::alloc_free_channel_locked() const
{
    for (int chn = 0; chn < MAX_CHN; chn++) {
        bool used = false;
        for (const auto &s : sessions_) {
            if (s.chn == chn) {
                used = true;
                break;
            }
        }
        if (!used)
            return chn;
    }
    return -1;
}

int uds_connection_manager::start()
{
    if (!mgr_ || running_.load())
        return -1;

    listen_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd_ < 0)
        return -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (uds_path_.size() >= sizeof(addr.sun_path))
        return -1;
    strncpy(addr.sun_path, uds_path_.c_str(), sizeof(addr.sun_path) - 1);
    unlink(uds_path_.c_str());
    if (bind(listen_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return -1;
    }
    if (listen(listen_fd_, 8) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return -1;
    }

    running_ = true;
    accept_thread_ = std::thread(&uds_connection_manager::accept_loop, this);
    cleanup_thread_ = std::thread(&uds_connection_manager::cleanup_loop, this);
    fprintf(stderr, "[uds_mgr] listening on %s, client sends 1 byte media type (video:0/'v', audio:1/'a'); channel auto-allocated\n", uds_path_.c_str());
    return 0;
}

void uds_connection_manager::stop()
{
    running_ = false;
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
    if (accept_thread_.joinable())
        accept_thread_.join();
    if (cleanup_thread_.joinable())
        cleanup_thread_.join();

    std::lock_guard<std::mutex> lock(sessions_mutex_);
    for (auto &s : sessions_) {
        if (s.type == media_type::video) {
            mgr_->remove_channel_listener(s.chn, s.stream.get());
            mgr_->stop_hdmi_video_channel(s.chn);
        } else {
            mgr_->stop_hdmi_audio_channel(s.chn);
        }
        s.stream->set_client_fd(-1);
        close(s.fd);
    }
    sessions_.clear();
}

void uds_connection_manager::accept_loop()
{
    while (running_.load() && listen_fd_ >= 0) {
        struct pollfd pfd = { listen_fd_, POLLIN, 0 };
        int r = poll(&pfd, 1, 500);
        if (r <= 0)
            continue;
        int client_fd = accept(listen_fd_, nullptr, nullptr);
        if (client_fd < 0)
            continue;

        media_type type;
        if (!read_media_type(client_fd, MEDIA_TYPE_READ_TIMEOUT_MS, &type)) {
            fprintf(stderr, "[uds_mgr] client fd=%d invalid media type or timeout, close\n", client_fd);
            close(client_fd);
            continue;
        }

        int chn = -1;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            chn = alloc_free_channel_locked();
        }
        if (chn < 0) {
            fprintf(stderr, "[uds_mgr] client fd=%d no free channel, close\n", client_fd);
            close(client_fd);
            continue;
        }

        uds_stream *stream = new uds_stream(client_fd);
        if (type == media_type::video) {
            mgr_->add_channel_listener(chn, stream);
            mgr_->start_hdmi_video_channel(chn);
        } else {
            mgr_->start_hdmi_audio_channel(chn);
        }

        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.emplace_back(client_fd, chn, type, stream);
        fprintf(stderr, "[uds_mgr] client fd=%d bound to chn=%d type=%s\n",
                client_fd, chn, type == media_type::video ? "video" : "audio");
    }
}

void uds_connection_manager::cleanup_loop()
{
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(CLEANUP_POLL_INTERVAL_MS));

        std::vector<int> to_remove_fds;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            if (sessions_.empty())
                continue;

            std::vector<struct pollfd> pfds;
            pfds.reserve(sessions_.size());
            for (const auto &s : sessions_)
                pfds.push_back({ s.fd, 0, 0 });

            int n = poll(pfds.data(), pfds.size(), 0);
            if (n <= 0)
                continue;

            for (size_t i = 0; i < pfds.size(); i++) {
                if (pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                    to_remove_fds.push_back(sessions_[i].fd);
            }
        }

        for (int fd : to_remove_fds) {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            auto it = std::find_if(sessions_.begin(), sessions_.end(),
                [fd](const session &s) { return s.fd == fd; });
            if (it != sessions_.end()) {
                if (it->type == media_type::video) {
                    mgr_->remove_channel_listener(it->chn, it->stream.get());
                    mgr_->stop_hdmi_video_channel(it->chn);
                } else {
                    mgr_->stop_hdmi_audio_channel(it->chn);
                }
                it->stream->set_client_fd(-1);
                close(it->fd);
                fprintf(stderr, "[uds_mgr] client fd=%d chn=%d type=%s disconnected\n",
                        it->fd, it->chn, it->type == media_type::video ? "video" : "audio");
                sessions_.erase(it);
            }
        }
    }
}
