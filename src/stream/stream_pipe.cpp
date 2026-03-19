#include "stream_pipe.hpp"

stream_pipe::stream_pipe()
    : head_(nullptr)
    , tail_(nullptr)
{
}

stream_pipe::~stream_pipe()
{
    if (tail_)
        tail_->stream_stop();
}

void stream_pipe::stream_connect(stream_base *stream, stream_listener *listener)
{
    if (!stream)
        return;
    if (!head_) {
        head_ = tail_ = stream;
        return;
    }
    stream_listener *l = listener ? listener : dynamic_cast<stream_listener *>(stream);
    if (l)
        tail_->stream_add_listener(l);
    tail_ = stream;
}

int stream_pipe::stream_pipe_start()
{
    if (!tail_)
        return -1;
    return tail_->stream_start();
}

int stream_pipe::stream_pipe_stop()
{
    if (!tail_)
        return 0;
    return tail_->stream_stop();
}
