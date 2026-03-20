#include "stream_base.hpp"
#include <algorithm>
#define MODULE_TAG "stream_base"

#include "log/log_tag.h"

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

    std::lock_guard<std::mutex> lock2(connects_mutex_);
    connects_.clear();
}

int stream_base::stream_start()
{
    if (status_.load() == STREAM_STATUS_RUNNING)
        return 0;
    running_ = true;
    th_ = std::thread(thread_entry, this);
    status_.store(STREAM_STATUS_RUNNING);
    WriteLog(LOG_DEBUG, "stream_start: %p status=%d running=%d",
             this, status_.load(), running_.load());
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
    WriteLog(LOG_DEBUG, "stream_stop: %p status=%d running=%d",
             this, status_.load(), running_.load());
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

void stream_base::stream_add_connect(stream_base *downstream)
{
    if (!downstream)
        return;
    std::lock_guard<std::mutex> lock(connects_mutex_);
    if (std::find(connects_.begin(), connects_.end(), downstream) == connects_.end())
        connects_.push_back(downstream);
}

void stream_base::stream_del_connect(stream_base *downstream)
{
    if (!downstream)
        return;
    std::lock_guard<std::mutex> lock(connects_mutex_);
    auto it = std::find(connects_.begin(), connects_.end(), downstream);
    if (it != connects_.end())
        connects_.erase(it);
}

// 每个 stream 实现自己的 stream_data_loop 中会调用 notify_listeners 通知每个 listeners 的 on_stream_data
void stream_base::notify_listeners(const char *data, int len)
{
    std::vector<stream_listener *> copy;
    {
        std::lock_guard<std::mutex> lock(listeners_mutex_);
        copy = listeners_;
    }
    for (auto *l : copy) {
        WriteLog(LOG_DEBUG, "notify_listeners: %p listener=%p", this, l);
        l->on_stream_data(data, len);
    }
}

// 每个 stream 去实现自己单独的 取流逻辑
void stream_base::thread_entry(stream_base *self)
{
    if (self)
        self->stream_data_loop();
}
