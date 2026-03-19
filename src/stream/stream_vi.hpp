#ifndef __STREAM_VI_HPP__
#define __STREAM_VI_HPP__

#include "stream_base.hpp"

/* 视频输入流：VI 数据源，可作为 pipe 的 head */
class stream_vi : public stream_base {
public:
    stream_vi();
    virtual ~stream_vi() = default;

protected:
    void stream_data_loop() override;
};

#endif /* __STREAM_VI_HPP__ */
