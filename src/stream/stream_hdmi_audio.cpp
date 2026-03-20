#include "stream_pipe.hpp"

stream_hdmi_audio::stream_hdmi_audio()
{
    stream_input_ = new stream_ai();
    //stream_process_ = new stream_base *[MAX_CHN]();
    stream_output_ = new stream_base *[MAX_CHN]();

    for (int i = 0; i < MAX_CHN; i++) {
        // stream_process_[i] = new stream_aenc(i);
        stream_output_[i] = new stream_aenc(i);
    }

    stream_pipe_init();
}
