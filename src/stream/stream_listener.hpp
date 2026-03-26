#ifndef __STREAM_LISTENER_HPP__
#define __STREAM_LISTENER_HPP__

/* 流数据监听者：接收流数据的回调接口 */
struct stream_listener {
    virtual ~stream_listener() = default;
    // 数据输入回调（上游节点调用此方法输入数据）
    virtual void on_data_input(const char *data, int len) = 0;
    // 数据输出回调（可选实现，用于监控/日志）
    virtual void on_data_output(const char *data, int len) { (void)data; (void)len; }
    // 兼容旧接口：默认实现调用 on_data_input
    virtual void on_stream_data(const char *data, int len) { on_data_input(data, len); }
};

#endif /* __STREAM_LISTENER_HPP__ */
