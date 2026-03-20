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
        }
    }
}

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
}
