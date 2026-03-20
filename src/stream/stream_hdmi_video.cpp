#include "stream_pipe.hpp"

stream_hdmi_video::stream_hdmi_video()
{
    /* stream_process_ / stream_output_ 必须先分配为 MAX_CHN 个槽位，否则对 [i] 写入会空指针崩溃 */
    stream_process_ = new stream_base *[MAX_CHN]();
    stream_output_ = new stream_base *[MAX_CHN]();

    stream_input_ = new stream_vi();
    stream_input_->stream_create();
}

stream_hdmi_video::~stream_hdmi_video()
{
    for (int i = 0; i < MAX_CHN; i++)
        stream_stop(i);

    if (stream_output_) {
        for (int i = 0; i < MAX_CHN; i++) {
            stream_input_->stream_del_connect(stream_output_[i]);
            delete stream_output_[i];
            stream_output_[i] = nullptr;
        }
        delete[] stream_output_;
        stream_output_ = nullptr;
    }

    // if (stream_process_) {
    //     for (int i = 0; i < MAX_CHN; i++) {
    //         delete stream_process_[i];
    //         stream_process_[i] = nullptr;
    //     }
    //     delete[] stream_process_;
    //     stream_process_ = nullptr;
    // }

    delete stream_input_;
    stream_input_ = nullptr;
}

void stream_hdmi_video::configure_venc_channels(int cfg_count)
{
    if (!stream_output_)
        return;
    if (cfg_count < 0)
        cfg_count = 0;
    if (cfg_count > MAX_CHN)
        cfg_count = MAX_CHN;

    /* 释放 cfg_count 之后可能残留的输出（理论上不会发生，防御性处理） */
    for (int i = cfg_count; i < MAX_CHN; i++) {
        if (stream_output_[i]) {
            stream_input_->stream_del_connect(stream_output_[i]);
            delete stream_output_[i];
            stream_output_[i] = nullptr;
        }
    }

    for (int i = 0; i < cfg_count; i++) {
        if (stream_output_[i])
            continue;
        stream_output_[i] = new stream_venc(i);
        stream_output_[i]->stream_create();
        stream_input_->stream_add_connect(stream_output_[i]);
    }
}
