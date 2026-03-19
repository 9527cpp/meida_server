#ifndef __STREAM_HDMI_VIDEO_HPP__
#define __STREAM_HDMI_VIDEO_HPP__

#include "stream_pipe.hpp"
#include "stream_vi.hpp"
#include "stream_venc.hpp"
#include "stream_listener.hpp"
#include "../mpi_ctx/mpi_ctx_intf.h"

/* 单路 HDMI 视频：1 个 VI + 最多 MAX_CHN 个 VENC 管道 */
class stream_hdmi_video {
public:
    stream_hdmi_video();
    ~stream_hdmi_video();

    void stream_pipe_create();
    void add_channel_listener(int chn, stream_listener *listener);
    void remove_channel_listener(int chn, stream_listener *listener);
    int stream_pipe_start(int chn);
    int stream_pipe_stop(int chn);

private:
    stream_pipe pipe_[MAX_CHN];
    stream_vi *vi_;
    stream_venc *venc_[MAX_CHN];
};

#endif /* __STREAM_HDMI_VIDEO_HPP__ */
