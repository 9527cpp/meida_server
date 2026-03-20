#include "stream_pipe.hpp"
#include "stream_base.hpp"

stream_pipe::stream_pipe()
    : stream_input_(nullptr)
    , stream_process_(nullptr)
    , stream_output_(nullptr)
{
}

stream_pipe::~stream_pipe()
{
    stream_pipe_deinit();
}

int stream_pipe::stream_start(int chn)
{
    if (chn < 0 || chn >= MAX_CHN)
        return -1;

    int ret = 0;

    if (stream_input_ && stream_input_->get_status() != STREAM_STATUS_RUNNING) {
        ret = stream_input_->stream_start();
        if (ret != 0)
            return ret;
    }


    if (stream_process_ && stream_process_[chn] && stream_process_[chn]->get_status() != STREAM_STATUS_RUNNING) {
        ret = stream_process_[chn]->stream_start();
        if (ret != 0)
            return ret;
    }

    if (stream_output_ && stream_output_[chn] && stream_output_[chn]->get_status() != STREAM_STATUS_RUNNING) {
        ret = stream_output_[chn]->stream_start();
    }

    return ret;
}

int stream_pipe::stream_stop(int chn)
{
    if (chn < 0 || chn >= MAX_CHN)
        return -1;

    int ret = 0;

    if (stream_output_ && stream_output_[chn] && stream_output_[chn]->get_status() == STREAM_STATUS_RUNNING) {
        ret = stream_output_[chn]->stream_stop();
        if (ret != 0)
            return ret;
    }

    if (stream_process_ && stream_process_[chn] && stream_process_[chn]->get_status() == STREAM_STATUS_RUNNING) {
        ret = stream_process_[chn]->stream_stop();
        if (ret != 0)
            return ret;
    }

    return ret;
}

void stream_pipe::add_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_CHN)
        return;
    if (stream_output_ && stream_output_[chn] && listener) {
        stream_output_[chn]->stream_add_listener(listener);
    }
}

void stream_pipe::remove_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_CHN)
        return;
    if (stream_output_ && stream_output_[chn] && listener) {
        stream_output_[chn]->stream_del_listener(listener);
    }
}

int stream_pipe::stream_pipe_init()
{
    int ret = 0;
    if (stream_input_ && stream_input_->get_status() != STREAM_STATUS_RUNNING) {
        ret = stream_input_->stream_start();
        if (ret != 0)
            return ret;
    }

    // for (int i = 0; i < MAX_CHN; i++) {
    //     if (stream_process_[i] && stream_process_[i]->get_status() != STREAM_STATUS_RUNNING) {
    //         ret = stream_process_[i]->stream_start();
    //         if (ret != 0)
    //             return ret;
    //     }
    // }

    for (int i = 0; i < MAX_CHN; i++) {
        if (stream_output_[i] && stream_output_[i]->get_status() != STREAM_STATUS_RUNNING) {
            ret = stream_output_[i]->stream_start();
            if (ret != 0)
                return ret;

            /* 将 output 节点连接到 input 节点 */
            stream_input_->stream_add_connect(stream_output_[i]);
        }
    }

    return ret;
}

void stream_pipe::stream_pipe_deinit()
{
    for (int i = 0; i < MAX_CHN; i++) {

        // if (stream_process_[i]) {
        //     delete stream_process_[i];
        //     stream_process_[i] = nullptr;
        // }

        if (stream_output_[i]) {
            stream_output_[i]->stream_stop();
            stream_input_->stream_del_connect(stream_output_[i]);
            delete stream_output_[i];
            stream_output_[i] = nullptr;
        }
    }

    if (stream_input_) {
        stream_input_->stream_stop();
        delete stream_input_;
        stream_input_ = nullptr;
    }
}