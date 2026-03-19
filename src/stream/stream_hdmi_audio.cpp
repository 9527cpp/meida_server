#include "stream_hdmi_audio.hpp"

stream_hdmi_audio::stream_hdmi_audio()
    : stream_(nullptr)
{
}

stream_hdmi_audio::~stream_hdmi_audio()
{
    stream_stop();
}

int stream_hdmi_audio::stream_start()
{
    (void)stream_;
    return 0;
}

int stream_hdmi_audio::stream_stop()
{
    return 0;
}
