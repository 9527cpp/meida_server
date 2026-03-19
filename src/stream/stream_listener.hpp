#ifndef __STREAM_LISTENER_HPP__
#define __STREAM_LISTENER_HPP__

/* 流数据监听者：接收流数据的回调接口 */
struct stream_listener {
    virtual ~stream_listener() = default;
    virtual void on_stream_data(const char *data, int len) = 0;
};

#endif /* __STREAM_LISTENER_HPP__ */
