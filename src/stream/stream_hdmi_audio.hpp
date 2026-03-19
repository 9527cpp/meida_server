#ifndef __STREAM_HDMI_AUDIO_HPP__
#define __STREAM_HDMI_AUDIO_HPP__

#include "stream_base.hpp"
#include "stream_listener.hpp"
#include "mpi_ctx_intf.h"

/* HDMI 音频流占位：可扩展为 ai + aenc 管道 */
class stream_hdmi_audio {
public:
    stream_hdmi_audio();
    ~stream_hdmi_audio();
    int stream_start();
    int stream_stop();
private:
    stream_base *stream_;
};

#endif /* __STREAM_HDMI_AUDIO_HPP__ */
