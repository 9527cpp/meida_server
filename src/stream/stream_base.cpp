#include "stream_base.hpp"
#include <algorithm>

stream_base::stream_base()
    : status_(STREAM_STATUS_STOPPED)
    , running_(false)
{
}

stream_base::~stream_base()
{
    stream_destroy();
}

int stream_base::stream_create()
{
    return 0;
}

void stream_base::stream_destroy()
{
    stream_stop();
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    listeners_.clear();
}

int stream_base::stream_start()
{
    if (status_.load() == STREAM_STATUS_RUNNING)
        return 0;
    running_ = true;
    th_ = std::thread(thread_entry, this);
    status_.store(STREAM_STATUS_RUNNING);
    return 0;
}

int stream_base::stream_stop()
{
    if (status_.load() == STREAM_STATUS_STOPPED)
        return 0;
    running_ = false;
    if (th_.joinable())
        th_.join();
    status_.store(STREAM_STATUS_STOPPED);
    return 0;
}

void stream_base::stream_add_listener(stream_listener *listener)
{
    if (!listener)
        return;
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end())
        listeners_.push_back(listener);
}

void stream_base::stream_del_listener(stream_listener *listener)
{
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    auto it = std::find(listeners_.begin(), listeners_.end(), listener);
    if (it != listeners_.end())
        listeners_.erase(it);
}

void stream_base::notify_listeners(const char *data, int len)
{
    std::vector<stream_listener *> copy;
    {
        std::lock_guard<std::mutex> lock(listeners_mutex_);
        copy = listeners_;
    }
    for (auto *l : copy)
        l->on_stream_data(data, len);
}

void stream_base::thread_entry(stream_base *self)
{
    if (self)
        self->stream_data_loop();
}
