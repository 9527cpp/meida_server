#include "uds_connection_manager.hpp"
#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"
#define MODULE_TAG "uds_conn_mgr"
#include "log/log_tag.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <string>

namespace {

const int REQUEST_READ_TIMEOUT_MS = 5000;
const int CLEANUP_POLL_INTERVAL_MS = 1000;
const int UDS_MAX_PACKS_NUM = 10;
const unsigned char UDS_MARK[4] = { 0xaa, 0xbb, 0xcc, 0xdd };
const int UDS_CMD_VIDEOSERVER_DEF = 0x2000;
const int VIDEOSERVER_CMD_ASSIGNVIDEO = UDS_CMD_VIDEOSERVER_DEF + 1;
const int VIDEOSERVER_CMD_ASSIGNAUDIO = UDS_CMD_VIDEOSERVER_DEF + 2;
const int VIDEOSERVER_CMD_GETRESOLUTION = UDS_CMD_VIDEOSERVER_DEF + 4;
const int VIDEOSERVER_CMD_GETIFRAME = UDS_CMD_VIDEOSERVER_DEF + 5;
const int VIDEOSERVER_CMD_SETSTATUSON = UDS_CMD_VIDEOSERVER_DEF + 6;
const int VIDEOSERVER_CMD_SETSTATUSOFF = UDS_CMD_VIDEOSERVER_DEF + 7;
const int VIDEOSERVER_CMD_GETSTATUS = UDS_CMD_VIDEOSERVER_DEF + 13;
const int VIDEOSERVER_CMD_ASSIGNAUDIO_MIC = UDS_CMD_VIDEOSERVER_DEF + 18;
const int VIDEOSERVER_CMD_GETONEFRAME = UDS_CMD_VIDEOSERVER_DEF + 20;
const int VIDEOSERVER_CMD_STOPVIDEO = UDS_CMD_VIDEOSERVER_DEF + 21;
const int VIDEOSERVER_CMD_GETFRAMEINFO = UDS_CMD_VIDEOSERVER_DEF + 22;
const int VIDEOSERVER_CMD_SETFRAMEINFO = UDS_CMD_VIDEOSERVER_DEF + 23;

struct uds_head {
    unsigned char mark[4];
    int cmd;
    int pack_count;
    uint32_t size[UDS_MAX_PACKS_NUM];
    int crc;
};

static bool wait_readable(int fd, int timeout_ms)
{
    struct pollfd pfd = { fd, POLLIN, 0 };
    return poll(&pfd, 1, timeout_ms) > 0;
}

static bool read_exact_with_timeout(int fd, void *buf, size_t len, int timeout_ms)
{
    unsigned char *p = static_cast<unsigned char *>(buf);
    size_t got = 0;
    while (got < len) {
        if (!wait_readable(fd, timeout_ms))
            return false;
        ssize_t n = recv(fd, p + got, len - got, 0);
        if (n <= 0)
            return false;
        got += (size_t)n;
    }
    return true;
}

static int extract_cmd_from_json_string(const std::string &payload)
{
    size_t k = payload.find("\"cmd\"");
    if (k == std::string::npos)
        return -1;
    k = payload.find(':', k);
    if (k == std::string::npos)
        return -1;
    k++;
    while (k < payload.size() && (payload[k] == ' ' || payload[k] == '\t' || payload[k] == '\r' || payload[k] == '\n'))
        k++;
    bool neg = false;
    if (k < payload.size() && payload[k] == '-') {
        neg = true;
        k++;
    }
    int val = 0;
    bool has_digit = false;
    while (k < payload.size() && payload[k] >= '0' && payload[k] <= '9') {
        has_digit = true;
        val = val * 10 + (payload[k] - '0');
        k++;
    }
    if (!has_digit)
        return -1;
    return neg ? -val : val;
}

} // namespace

uds_connection_manager::uds_connection_manager(media_event_sink *sink, const std::string &uds_path)
    : sink_(sink)
    , uds_path_(uds_path)
    , listen_fd_(-1)
    , running_(false)
{
}

uds_connection_manager::~uds_connection_manager()
{
    stop();
}

bool uds_connection_manager::parse_connect_request(int fd, int timeout_ms, connect_request *out_req)
{
    if (!out_req)
        return false;

    unsigned char mark[4] = { 0 };
    ssize_t n = recv(fd, mark, sizeof(mark), MSG_PEEK);
    if (n == (ssize_t)sizeof(mark) && memcmp(mark, UDS_MARK, sizeof(UDS_MARK)) == 0) {
        uds_head head;
        if (!read_exact_with_timeout(fd, &head, sizeof(head), timeout_ms))
            return false;
        if (memcmp(head.mark, UDS_MARK, sizeof(UDS_MARK)) != 0)
            return false;
        if (head.pack_count < 1 || head.pack_count > UDS_MAX_PACKS_NUM)
            return false;
        if (head.size[0] > 1024 * 1024)
            return false;

        std::string payload;
        payload.resize(head.size[0]);
        if (head.size[0] > 0 && !read_exact_with_timeout(fd, &payload[0], head.size[0], timeout_ms))
            return false;

        for (int i = 1; i < head.pack_count; i++) {
            uint32_t skip = head.size[i];
            while (skip > 0) {
                char tmp[256];
                size_t chunk = skip > sizeof(tmp) ? sizeof(tmp) : skip;
                if (!read_exact_with_timeout(fd, tmp, chunk, timeout_ms))
                    return false;
                skip -= (uint32_t)chunk;
            }
        }

        int cmd = head.cmd;
        int cmd_in_json = extract_cmd_from_json_string(payload);
        if (cmd_in_json > 0)
            cmd = cmd_in_json;

        media_type type;
        if (!parse_media_type_from_cmd(cmd, &type))
            return false;

        out_req->type = type;
        out_req->cmd = cmd;
        out_req->json_payload = payload;
        return true;
    }

    if (!wait_readable(fd, timeout_ms))
        return false;
    unsigned char t = 0xff;
    n = recv(fd, &t, 1, 0);
    if (n != 1)
        return false;
    if (t == 0 || t == 'v' || t == 'V') {
        out_req->type = media_type::video;
        out_req->cmd = VIDEOSERVER_CMD_ASSIGNVIDEO;
        out_req->json_payload = "{\"cmd\":2001}";
        return true;
    }
    if (t == 1 || t == 'a' || t == 'A') {
        out_req->type = media_type::audio;
        out_req->cmd = VIDEOSERVER_CMD_ASSIGNAUDIO;
        out_req->json_payload = "{\"cmd\":2002}";
        return true;
    }
    return false;
}

bool uds_connection_manager::parse_media_type_from_cmd(int cmd, media_type *out_type)
{
    if (!out_type)
        return false;
    switch (cmd) {
    case VIDEOSERVER_CMD_ASSIGNVIDEO:
        *out_type = media_type::video;
        return true;
    case VIDEOSERVER_CMD_ASSIGNAUDIO:
    case VIDEOSERVER_CMD_ASSIGNAUDIO_MIC:
        *out_type = media_type::audio;
        return true;
    default:
        return false;
    }
}

const char *uds_connection_manager::cmd_name(int cmd)
{
    switch (cmd) {
    case VIDEOSERVER_CMD_ASSIGNVIDEO: return "ASSIGNVIDEO";
    case VIDEOSERVER_CMD_ASSIGNAUDIO: return "ASSIGNAUDIO";
    case VIDEOSERVER_CMD_GETIFRAME: return "GETIFRAME";
    case VIDEOSERVER_CMD_GETRESOLUTION: return "GETRESOLUTION";
    case VIDEOSERVER_CMD_SETSTATUSON: return "SETSTATUSON";
    case VIDEOSERVER_CMD_SETSTATUSOFF: return "SETSTATUSOFF";
    case VIDEOSERVER_CMD_GETSTATUS: return "GETSTATUS";
    case VIDEOSERVER_CMD_ASSIGNAUDIO_MIC: return "ASSIGNAUDIO_MIC";
    case VIDEOSERVER_CMD_GETONEFRAME: return "GETONEFRAME";
    case VIDEOSERVER_CMD_STOPVIDEO: return "STOPVIDEO";
    case VIDEOSERVER_CMD_GETFRAMEINFO: return "GETFRAMEINFO";
    case VIDEOSERVER_CMD_SETFRAMEINFO: return "SETFRAMEINFO";
    default: return "UNKNOWN";
    }
}

int uds_connection_manager::alloc_free_channel_locked(media_type type) const
{
    struct mpi_intf *mpi = mpi_intf_get_instance();
    struct mpi_ctx *ctx_data = mpi ? mpi->ctx_data : NULL;

    for (int chn = 0; chn < MAX_CHN; chn++) {
        if (type == media_type::video && ctx_data) {
            if (ctx_data->v_ctx.venc[chn].enable == 0)
                continue;
        }

        bool used = false;
        for (const auto &s : sessions_) {
            if (s.type == type && s.chn == chn) {
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
    if (!sink_ || running_.load())
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
    WriteLog(LOG_INFO, "listening on %s, parse libuds cmd+json (fallback legacy 1-byte media type), channel auto-allocated",
             uds_path_.c_str());
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
            sink_->post_event({ media_event_type::stop_video, s.chn, s.stream.get() });
        } else {
            sink_->post_event({ media_event_type::stop_audio, s.chn, nullptr });
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

        connect_request req;
        if (!parse_connect_request(client_fd, REQUEST_READ_TIMEOUT_MS, &req)) {
            WriteLog(LOG_ERROR, "client fd=%d parse request failed or timeout, close", client_fd);
            close(client_fd);
            continue;
        }

        int chn = -1;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            chn = alloc_free_channel_locked(req.type);
        }
        if (chn < 0) {
            WriteLog(LOG_ERROR, "client fd=%d no free channel, close", client_fd);
            close(client_fd);
            continue;
        }

        uds_stream *stream = new uds_stream(client_fd);
        if (req.type == media_type::video) {
            sink_->post_event({ media_event_type::start_video, chn, stream });
        } else {
            sink_->post_event({ media_event_type::start_audio, chn, nullptr });
        }

        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.emplace_back(client_fd, chn, req.type, stream);
        WriteLog(LOG_INFO, "client fd=%d cmd=%d(%s) chn=%d type=%s json_len=%zu",
                 client_fd, req.cmd, cmd_name(req.cmd), chn,
                 req.type == media_type::video ? "video" : "audio",
                 req.json_payload.size());
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
                    sink_->post_event({ media_event_type::stop_video, it->chn, it->stream.get() });
                } else {
                    sink_->post_event({ media_event_type::stop_audio, it->chn, nullptr });
                }
                it->stream->set_client_fd(-1);
                close(it->fd);
                WriteLog(LOG_INFO, "client fd=%d chn=%d type=%s disconnected",
                         it->fd, it->chn, it->type == media_type::video ? "video" : "audio");
                sessions_.erase(it);
            }
        }
    }
}
