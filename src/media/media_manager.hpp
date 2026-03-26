#ifndef __MEDIA_MANAGER_HPP__
#define __MEDIA_MANAGER_HPP__

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "stream/stream_hdmi_video.hpp"
#include "stream/stream_hdmi_audio.hpp"
#include "stream/stream_listener.hpp"
#include "hw_check/hw_check.hpp"
#include "media_events.hpp"

/* 全局 media 管理：各路pipe流、事件循环与处理、硬件检测 */
class media_manager : public media_event_sink, public hw_check_listener {
public:
    media_manager();
    ~media_manager();

    /* 初始化 */
    int init();

    /* 释放资源 */
    void deinit();

    /* 发送事件 */
    void post_event(const media_event &ev) override;

    /* 硬件事件回调 */
    void on_hw_check_notify(hw_type type, hw_event ev) override;

private:
    void event_loop();

    /* 各路pipe流 */
    std::unique_ptr<stream_hdmi_video> hdmi_video_pipe_;
    std::unique_ptr<stream_hdmi_audio> hdmi_audio_pipe_;
    // std::unique_ptr<stream_hdmi_video> usb_video_pipe_;
    // std::unique_ptr<stream_hdmi_audio> mic_audio_pipe_;

    /* 硬件检测 */
    std::unique_ptr<hw_check> hdmi_check_;
    std::unique_ptr<hw_check> usb_check_;

    /* 消息相关: 事件循环、事件队列、事件处理 */
    std::mutex ev_mutex_;
    std::condition_variable ev_cv_;
    std::queue<media_event> ev_q_;
    bool ev_running_;
    std::thread ev_thread_;
};

#endif /* __MEDIA_MANAGER_HPP__ */
