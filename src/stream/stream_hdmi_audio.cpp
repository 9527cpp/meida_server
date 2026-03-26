#include "stream_hdmi_audio.hpp"
#include "stream_base.hpp"
#include "log/log_tag.h"

#define MODULE_TAG "stream_hdmi_audio"

stream_hdmi_audio::stream_hdmi_audio()
{
    init_streams();
}

stream_hdmi_audio::~stream_hdmi_audio()
{
    deinit_streams();
}

void stream_hdmi_audio::init_streams()
{
    input_ = new stream_ai();

    for (int i = 0; i < MAX_AENC_CHN; i++) {
        stream_base *aenc = new stream_aenc(i);
        outputs_.push_back(aenc);
        input_->add_stream(aenc);
    }
}

void stream_hdmi_audio::deinit_streams()
{
    for (auto *out : outputs_) {
        delete out;
    }
    outputs_.clear();

    if (input_) {
        delete input_;
        input_ = nullptr;
    }
}

int stream_hdmi_audio::start(int chn)
{
    if (chn < 0 || chn >= MAX_AENC_CHN)
        return -1;

    int ret = 0;

    if (input_ && input_->get_status() != STREAM_STATUS_RUNNING) {
        ret = input_->stream_start();
        if (ret != 0)
            return ret;
    }

    if (chn < (int)outputs_.size() && outputs_[chn]) {
        ret = outputs_[chn]->stream_start();
    }

    return ret;
}

int stream_hdmi_audio::stop(int chn)
{
    if (chn < 0 || chn >= MAX_AENC_CHN)
        return -1;

    int ret = 0;

    if (chn < (int)outputs_.size() && outputs_[chn]) {
        ret = outputs_[chn]->stream_stop();
    }

    return ret;
}

void stream_hdmi_audio::add_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_AENC_CHN)
        return;
    if (chn < (int)outputs_.size() && outputs_[chn] && listener) {
        outputs_[chn]->stream_add_listener(listener);
    }
}

void stream_hdmi_audio::remove_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_AENC_CHN)
        return;
    if (chn < (int)outputs_.size() && outputs_[chn] && listener) {
        outputs_[chn]->stream_del_listener(listener);
    }
}

void stream_hdmi_audio::add_process_stream(int chn, stream_base *stream)
{
    if (chn < 0 || chn >= MAX_AENC_CHN || !stream)
        return;
    if (chn < (int)outputs_.size()) {
        process_streams_.push_back(stream);
    }
}

void stream_hdmi_audio::add_output_stream(int chn, stream_base *stream)
{
    if (chn < 0 || chn >= MAX_AENC_CHN || !stream)
        return;
    if (chn < (int)outputs_.size() && outputs_[chn]) {
        outputs_[chn]->add_stream(stream);
    }
}

void stream_hdmi_audio::stream_data_loop()
{
    while (is_running()) {
        usleep(100000);
    }
}
