#ifndef __MEDIA_MANAGER_HPP__
#define __MEDIA_MANAGER_HPP__

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "stream/stream_hdmi_video.hpp"
#include "stream/stream_hdmi_audio.hpp"
#include "stream/stream_listener.hpp"
#include "hw_check/hw_check_intf.h"
#include "media_events.hpp"

/* hw_check 插件实例抽象（不直接依赖 C++ 类） */
struct hw_check_instance {
    void *so_handle;                  /* dlopen 句柄 */
    struct hw_check_opt *opt;         /* 函数指针集合 */
    struct hw_check_ctx *ctx;         /* 实例上下文 */
};

/* 全局 media 管理：各路pipe流、事件循环与处理、硬件检测 */
class media_manager : public media_event_sink {
public:
    explicit media_manager(const char *hw_plugin_dir = nullptr);
    ~media_manager();

    /* 初始化 */
    int init();

    /* 释放资源 */
    void deinit();

    /* 发送事件 */
    void post_event(const media_event &ev) override;

    /* 硬件事件回调（由 C 回调桥接函数调用） */
    void on_hw_check_notify(enum hw_check_type type, enum hw_check_event ev);

private:
    void event_loop();
    int load_hw_check_plugins(const char *dir);
    void unload_hw_check_plugins();

    /* 各路pipe流 */
    std::unique_ptr<stream_hdmi_video> hdmi_video_pipe_;
    std::unique_ptr<stream_hdmi_audio> hdmi_audio_pipe_;
    // std::unique_ptr<stream_hdmi_video> usb_video_pipe_;
    // std::unique_ptr<stream_hdmi_audio> mic_audio_pipe_;

    /* 硬件检测插件实例 */
    std::vector<hw_check_instance> hw_check_instances_;

    /* 消息相关: 事件循环、事件队列、事件处理 */
    std::mutex ev_mutex_;
    std::condition_variable ev_cv_;
    std::queue<media_event> ev_q_;
    bool ev_running_;
    std::thread ev_thread_;
};

#endif /* __MEDIA_MANAGER_HPP__ */
