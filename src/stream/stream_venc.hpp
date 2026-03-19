#ifndef __STREAM_VENC_HPP__
#define __STREAM_VENC_HPP__

#include "stream_base.hpp"
#include "stream_listener.hpp"

/* 视频编码流：可从 MPI 取编码数据并转发给监听者；也可作为 pipe 中段接收上一级数据 */
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

#endif /* __STREAM_VENC_HPP__ */
