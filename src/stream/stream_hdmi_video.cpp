#include "stream_hdmi_video.hpp"

stream_hdmi_video::stream_hdmi_video()
    : vi_(nullptr)
{
    for (int i = 0; i < MAX_CHN; i++)
        venc_[i] = nullptr;
}

stream_hdmi_video::~stream_hdmi_video()
{
    for (int i = 0; i < MAX_CHN; i++)
        stream_pipe_stop(i);
    delete vi_;
    vi_ = nullptr;
    for (int i = 0; i < MAX_CHN; i++) {
        delete venc_[i];
        venc_[i] = nullptr;
    }
}

void stream_hdmi_video::stream_pipe_create()
{
    vi_ = new stream_vi();
    vi_->stream_create();

    for (int i = 0; i < MAX_CHN; i++) {
        venc_[i] = new stream_venc(i);
        venc_[i]->stream_create();
        pipe_[i].stream_connect(vi_);
        pipe_[i].stream_connect(venc_[i], venc_[i]);
    }
}

void stream_hdmi_video::add_channel_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_CHN || !venc_[chn] || !listener)
        return;
    venc_[chn]->stream_add_listener(listener);
}

void stream_hdmi_video::remove_channel_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_CHN || !venc_[chn] || !listener)
        return;
    venc_[chn]->stream_del_listener(listener);
}

int stream_hdmi_video::stream_pipe_start(int chn)
{
    if (chn < 0 || chn >= MAX_CHN)
        return -1;
    return pipe_[chn].stream_pipe_start();
}

int stream_hdmi_video::stream_pipe_stop(int chn)
{
    if (chn < 0 || chn >= MAX_CHN)
        return -1;
    return pipe_[chn].stream_pipe_stop();
}
