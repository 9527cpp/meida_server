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

    /* 启动某个pipe流, 参数表示 output 的chn, 如果前级没启动, 则会默认启动前级节点 */
    virtual int stream_start(int chn);

    /* 停止某个pipe流, 参数表示 output 的chn, 除了不停 input 节点, 其他在pipe上的节点都会停止 */
    virtual int stream_stop(int chn);

    /* 添加某个pipe流的输出监听器, 参数表示 output 的chn */
    virtual void add_listener(int chn, stream_listener *listener);

    /* 删除某个pipe流的输出监听器, 参数表示 output 的chn */
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

class stream_venc : public stream_base {
public:
    stream_venc(int chn);
    virtual ~stream_venc() = default;

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

class stream_aenc : public stream_base {
public:
    stream_aenc(int chn);
    virtual ~stream_aenc() = default;

protected:
    void stream_data_loop() override;

private:
    int chn_;
};

class stream_hdmi_video : public stream_pipe {
public:
    stream_hdmi_video();
    ~stream_hdmi_video();
    /* 根据 mpi_ctx.v_ctx.venc_cfg_count 只创建对应数量的 stream_venc（逻辑索引 0..cfg_count-1） */
    void configure_venc_channels(int cfg_count);
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
