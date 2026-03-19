#ifndef __MEDIA_MANAGER_HPP__
#define __MEDIA_MANAGER_HPP__

#include "../stream/stream_hdmi_video.hpp"
#include "../stream/stream_hdmi_audio.hpp"
#include "../stream/stream_usb_video.hpp"
#include "../stream/stream_mic_audio.hpp"
#include "../stream/stream_listener.hpp"
#include "../hw_check/hw_check.hpp"
#include "../hw_check/hdmi_check.hpp"
#include "../hw_check/usb_check.hpp"

/* 全局 media 管理：MPI/ctx 单例、各路流、监听者、硬件检测 */
class media_manager {
public:
    media_manager();
    ~media_manager();

    /* 初始化：需在设置 mpi/ctx 后调用 */
    int init();
    void deinit();

    /* 流启停 */
    int start_hdmi_video_channel(int chn);
    int stop_hdmi_video_channel(int chn);
    int start_hdmi_audio_channel(int chn);
    int stop_hdmi_audio_channel(int chn);

    /* 按通道添加/移除监听者（多客户端时每个连接绑定一个 chn，如 chn0=H264 chn1=H265） */
    void add_channel_listener(int chn, stream_listener *listener);
    void remove_channel_listener(int chn, stream_listener *listener);

    void set_listener(stream_listener *listener);
    stream_listener *get_listener() const { return listener_; }

    hw_check *get_hdmi_check() { return hdmi_check_; }
    hw_check *get_usb_check() { return usb_check_; }

    stream_hdmi_video *get_hdmi_video() { return &s_hdmi_video_; }
    stream_hdmi_audio *get_hdmi_audio() { return &s_hdmi_audio_; }
    stream_usb_video *get_usb_video() { return &s_usb_video_; }
    stream_mic_audio *get_mic_audio() { return &s_mic_audio_; }

private:
    stream_hdmi_video s_hdmi_video_;
    stream_hdmi_audio s_hdmi_audio_;
    stream_usb_video s_usb_video_;
    stream_mic_audio s_mic_audio_;

    stream_listener *listener_;
    hw_check *hdmi_check_;
    hw_check *usb_check_;
};

#endif /* __MEDIA_MANAGER_HPP__ */
