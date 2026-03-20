#include "media_manager.hpp"
#define MODULE_TAG "media_manager"

#include "log/log_tag.h"

media_manager::media_manager()
    : ev_running_(false)
{
    hdmi_video_pipe_.reset(new stream_hdmi_video());
    hdmi_audio_pipe_.reset(new stream_hdmi_audio());
    // usb_video_pipe_.reset(new stream_usb_video());
    // mic_audio_pipe_.reset(new stream_mic_audio());

    // hdmi_check_.reset(new hdmi_check(this, false));
    // usb_check_.reset(new usb_check(this, false));
    hdmi_check_.reset(new hdmi_check(this, true));
    usb_check_.reset(new usb_check(this, true));

    init();
}

media_manager::~media_manager()
{
    deinit();
}

int media_manager::init()
{
    {
        std::lock_guard<std::mutex> lock(ev_mutex_);
        ev_running_ = true;
    }
    ev_thread_ = std::thread(&media_manager::event_loop, this);
    return 0;
}

void media_manager::deinit()
{
    {
        std::lock_guard<std::mutex> lock(ev_mutex_);
        if (ev_running_) {
            ev_running_ = false;
            ev_cv_.notify_all();
        }
    }
    if (ev_thread_.joinable())
        ev_thread_.join();
}


void media_manager::post_event(const media_event &ev)
{
    {
        std::lock_guard<std::mutex> lock(ev_mutex_);
        if (!ev_running_)
            return;
        ev_q_.push(ev);
    }
    ev_cv_.notify_one();
}

/*
    目前有两个地方会发事件
    1: hw_check 事件回调, 收到事件后 发消息给event_loop
    2: uds 连接管理, 收到连接请求后 发消息给event_loop
    从 队列中 取事件, 处理事件: 如启用/停止 视频/音频 流
*/
void media_manager::event_loop()
{
    while (true) {
        media_event ev;
        {
            std::unique_lock<std::mutex> lock(ev_mutex_);
            ev_cv_.wait(lock, [this]() { return !ev_running_ || !ev_q_.empty(); });
            if (!ev_running_ && ev_q_.empty())
                break;
            ev = ev_q_.front();
            ev_q_.pop();
        }

        if (ev.chn < 0 || ev.chn >= MAX_CHN) {
            WriteLog(LOG_ERROR, "invalid channel: chn=%d", ev.chn);
            continue;
        }

        switch (ev.type) {
        case media_event_type::start_video:
            hdmi_video_pipe_->stream_start(ev.chn);
            break;
        case media_event_type::stop_video:
            hdmi_video_pipe_->stream_stop(ev.chn);
            break;
        case media_event_type::assign_video:
            hdmi_video_pipe_->add_listener(ev.chn, ev.listener);
            break;
        case media_event_type::unassign_video:
            hdmi_video_pipe_->remove_listener(ev.chn, ev.listener);
            break;
        case media_event_type::start_audio:
            hdmi_audio_pipe_->stream_start(ev.chn);
            break;
        case media_event_type::stop_audio:
            hdmi_audio_pipe_->stream_stop(ev.chn);
            break;
        case media_event_type::assign_audio:
            hdmi_audio_pipe_->add_listener(ev.chn, ev.listener);
            break;
        case media_event_type::unassign_audio:
            hdmi_audio_pipe_->remove_listener(ev.chn, ev.listener);
            break;
        /* TODO: 其他事件处理 */
        }
    }
}

/*
    hw_check 事件回调, 收到事件后 发消息给event_loop, 让event_loop去处理对应的流事件, 而不是在此回调中直接调用 stream_start 或 stream_stop 
*/
void media_manager::on_hw_check_notify(hw_type type, hw_event ev)
{
    const char *t = "unknown";
    switch (type) {
    case hw_type::hdmi: t = "hdmi"; break;
    case hw_type::usb: t = "usb"; break;
    case hw_type::mic: t = "mic"; break;
    }

    const char *e = (ev == hw_event::plug_in) ? "plug_in" : "plug_out";
    WriteLog(LOG_INFO, "%s %s", t, e);

    if (type == hw_type::hdmi) {
        /* HDMI plug_in/out：对所有 enable 的 venc 通道分别 start/stop */
        for (int i = 0; i < MAX_CHN; i++) {
            media_event ev_media;
            ev_media.type = (ev == hw_event::plug_in) ? media_event_type::start_video
                                                      : media_event_type::stop_video;
            ev_media.chn = i;
            ev_media.listener = nullptr;
            post_event(ev_media);
        }
        return;
    }

    /* 
    * TODO:
    * 其它硬件类型保持现有行为（如需同样批量处理可再继续补齐） 
    */
    // media_event ev_media;
    // ev_media.type = (ev == hw_event::plug_in) ? media_event_type::start_video
    //                                           : media_event_type::stop_video;
    // ev_media.chn = 0;
    // ev_media.listener = nullptr;
    // post_event(ev_media);

}
