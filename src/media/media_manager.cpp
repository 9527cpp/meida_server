#include "media_manager.hpp"
#include "../mpi/mpi_intf.h"
#include "../mpi_ctx/mpi_ctx_intf.h"

media_manager::media_manager()
    : listener_(nullptr)
    , hdmi_check_(nullptr)
    , usb_check_(nullptr)
{
    hdmi_check_ = new hdmi_check(false);
    usb_check_ = new usb_check(false);
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
    s_hdmi_video_.stream_pipe_create();
    return 0;
}

void media_manager::deinit()
{
    for (int i = 0; i < MAX_CHN; i++) {
        if (listener_)
            s_hdmi_video_.remove_channel_listener(i, listener_);
        s_hdmi_video_.stream_pipe_stop(i);
    }
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (mpi)
        mpi_intf_deinit(mpi);
}

int media_manager::start_hdmi_video_channel(int chn)
{
    if (listener_)
        s_hdmi_video_.add_channel_listener(chn, listener_);
    return s_hdmi_video_.stream_pipe_start(chn);
}

int media_manager::stop_hdmi_video_channel(int chn)
{
    return s_hdmi_video_.stream_pipe_stop(chn);
}

int media_manager::start_hdmi_audio_channel(int chn)
{
    (void)chn;
    return s_hdmi_audio_.stream_start();
}

int media_manager::stop_hdmi_audio_channel(int chn)
{
    (void)chn;
    return s_hdmi_audio_.stream_stop();
}

void media_manager::add_channel_listener(int chn, stream_listener *listener)
{
    if (listener)
        s_hdmi_video_.add_channel_listener(chn, listener);
}

void media_manager::remove_channel_listener(int chn, stream_listener *listener)
{
    if (listener)
        s_hdmi_video_.remove_channel_listener(chn, listener);
}

void media_manager::set_listener(stream_listener *listener)
{
    listener_ = listener;
}
