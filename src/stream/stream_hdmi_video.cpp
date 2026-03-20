#include "stream_pipe.hpp"
#define MODULE_TAG "stream_hdmi_video"
#include "log/log_tag.h"

stream_hdmi_video::stream_hdmi_video()
{
    stream_input_ = new stream_vi();
    //stream_process_ = new stream_base *[MAX_CHN]();
    stream_output_ = new stream_base *[MAX_CHN]();

    for (int i = 0; i < MAX_CHN; i++) {
        // stream_process_[i] = new stream_vpss(i);
        stream_output_[i] = new stream_venc(i);
    }

    stream_pipe_init();
}