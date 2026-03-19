#ifndef __STREAM_PIPE_HPP__
#define __STREAM_PIPE_HPP__

#include "stream_base.hpp"
#include "stream_listener.hpp"
#include "../mpi_ctx/mpi_ctx_intf.h"

class stream_base;

/**
 * 流媒体管道抽象基类。
 *
 * - stream_input：单一输入节点（如 VI / AI）
 * - stream_process：按通道的中间节点数组，可为 nullptr 或每通道 nullptr（无处理）
 * - stream_output：按通道的输出节点数组（如 VENC / AENC），多路编码时常用
 *
 * 子类在构造中创建具体 stream_base 后，调用 bind_pipe(...) 绑定上述指针。
 */
class stream_pipe {
public:
    stream_pipe();
    virtual ~stream_pipe() = default;

    virtual int stream_start(int chn);
    virtual int stream_stop(int chn);
    virtual void add_listener(int chn, stream_listener *listener);
    virtual void remove_listener(int chn, stream_listener *listener);

protected:
    stream_base *stream_input_{nullptr};
    stream_base **stream_process_{nullptr};
    stream_base **stream_output_{nullptr};
};

/* 视频输入流：VI 数据源，可作为 pipe 的 head */
class stream_vi : public stream_base {
public:
    stream_vi();
    virtual ~stream_vi() = default;

protected:
    void stream_data_loop() override;
};

/* 视频编码流：可从 MPI 取编码数据并转发给监听者；也可作为 pipe
 * 中段接收上一级数据 */
class stream_venc : public stream_base, public stream_listener {
public:
    stream_venc(int chn);
    virtual ~stream_venc() = default;

    void on_stream_data(const char *data, int len) override;

protected:
    void stream_data_loop() override;

private:
    int chn_;
};


/* 音频输入流：AI 数据源，占位；当前编码节点直接从 MPI AENC 拉取数据 */
class stream_ai : public stream_base {
public:
    stream_ai();
    virtual ~stream_ai() = default;

protected:
    void stream_data_loop() override;
};

class stream_aenc : public stream_base, public stream_listener {
public:
    stream_aenc(int chn);
    virtual ~stream_aenc() = default;

    void on_stream_data(const char *data, int len) override;

protected:
    void stream_data_loop() override;

private:
    int chn_;
};

class stream_hdmi_video : public stream_pipe {
public:
    stream_hdmi_video();
    ~stream_hdmi_video();
};

class stream_hdmi_audio : public stream_pipe {
public:
    stream_hdmi_audio();
    ~stream_hdmi_audio();
};

class stream_usb_video : public stream_pipe {
public:
    stream_usb_video();
    ~stream_usb_video();
};

class stream_mic_audio : public stream_pipe {
public:
    stream_mic_audio();
    ~stream_mic_audio();
};

#endif /* __STREAM_PIPE_HPP__ */
