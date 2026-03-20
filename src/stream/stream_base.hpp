#ifndef __STREAM_BASE_HPP__
#define __STREAM_BASE_HPP__

#include "stream_listener.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

/* 流状态 */
enum stream_status {
    STREAM_STATUS_STOPPED = 0,
    STREAM_STATUS_RUNNING = 1
};

enum stream_type {
    STREAM_TYPE_VI = 0,
    STREAM_TYPE_VENC = 1,
    STREAM_TYPE_AI = 2,
    STREAM_TYPE_AENC = 3,
};

/* 基本流：负责 MPI 取数线程、监听者列表、启停 */
class stream_base {
public:
    stream_base();
    virtual ~stream_base();

    /* 创建流（线程/资源创建，MPI 初始化） */
    int stream_create();
    /* 销毁流（线程销毁，MPI 反初始化，移除所有监听者） */
    void stream_destroy();

    /* 启停 */
    int stream_start();
    int stream_stop();

    void stream_add_listener(stream_listener *listener);
    void stream_del_listener(stream_listener *listener);

    stream_status get_status() const { return status_.load(); }

protected:
    /* 子类实现：从 MPI 取数据并通知监听者 */
    virtual void stream_data_loop() = 0;

    void notify_listeners(const char *data, int len);
    bool is_running() const { return running_.load(); }

private:
    static void thread_entry(stream_base *self);

    std::vector<stream_listener *> listeners_;
    std::mutex listeners_mutex_;
    std::thread th_;
    std::atomic<stream_status> status_;
    std::atomic<bool> running_;
};

#endif /* __STREAM_BASE_HPP__ */
