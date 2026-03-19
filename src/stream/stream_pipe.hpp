#ifndef __STREAM_PIPE_HPP__
#define __STREAM_PIPE_HPP__

#include "stream_base.hpp"

/* 管道流：将多个 stream_base 串联，head -> ... -> tail，数据从 head 流向 tail */
class stream_pipe {
public:
    stream_pipe();
    ~stream_pipe();

    /* 若 listener 为 nullptr，则使用 dynamic_cast<stream_listener*>(stream) */
    void stream_connect(stream_base *stream, stream_listener *listener = nullptr);
    int stream_pipe_start();
    int stream_pipe_stop();

    stream_base *head() { return head_; }
    stream_base *tail() { return tail_; }

private:
    stream_base *head_;
    stream_base *tail_;
};

#endif /* __STREAM_PIPE_HPP__ */
