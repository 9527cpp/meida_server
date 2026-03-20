#include "media_manager.hpp"
#include "../mpi/mpi_intf.h"
#include "../mpi_ctx/mpi_ctx_intf.h"

media_manager::media_manager()
    : hdmi_check_(nullptr)
    , usb_check_(nullptr)
    , ev_running_(false)
{
    hdmi_video_pipe_.reset(new stream_hdmi_video());
    hdmi_audio_pipe_.reset(new stream_hdmi_audio());

    // hdmi_check_ = new hdmi_check(this, false);
    // usb_check_ = new usb_check(this, false);
    hdmi_check_ = new hdmi_check(this, true);
    usb_check_ = new usb_check(this, true);
}

media_manager::~media_manager()
{
    deinit();
    delete hdmi_check_;
    delete usb_check_;
    hdmi_check_ = usb_check_ = nullptr;
}

int media_manager::init()
{
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (!mpi)
        return -1;
    if (mpi_intf_init(mpi) != 0)
        return -1;

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

    for (int i = 0; i < MAX_CHN; i++) {
        hdmi_video_pipe_->stream_stop(i);
        hdmi_audio_pipe_->stream_stop(i);
    }
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (mpi)
        mpi_intf_deinit(mpi);
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

        switch (ev.type) {
        case media_event_type::start_video:
            if (ev.listener)
                hdmi_video_pipe_->add_listener(ev.chn, ev.listener);
            hdmi_video_pipe_->stream_start(ev.chn);
            break;
        case media_event_type::stop_video:
            if (ev.listener)
                hdmi_video_pipe_->remove_listener(ev.chn, ev.listener);
            hdmi_video_pipe_->stream_stop(ev.chn);
            break;
        case media_event_type::start_audio:
            if (ev.listener)
                hdmi_audio_pipe_->add_listener(ev.chn, ev.listener);
            hdmi_audio_pipe_->stream_start(ev.chn);
            break;
        case media_event_type::stop_audio:
            if (ev.listener)
                hdmi_audio_pipe_->remove_listener(ev.chn, ev.listener);
            hdmi_audio_pipe_->stream_stop(ev.chn);
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
    fprintf(stderr, "[hw_check] %s %s\n", t, e);

    media_event ev_media;
    ev_media.type = media_event_type::start_video;
    ev_media.chn = 0;
    ev_media.listener = nullptr;
    if (type == hw_type::hdmi) {
        if (ev == hw_event::plug_in) {
            ev_media.type = media_event_type::start_video;
        } else {
            ev_media.type = media_event_type::stop_video;
        }
    } else if (type == hw_type::usb) {
        if (ev == hw_event::plug_in) {
            ev_media.type = media_event_type::start_video;
        } else {
            ev_media.type = media_event_type::stop_video;
        }
    }
    post_event(ev_media);
}
