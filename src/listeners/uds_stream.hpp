#ifndef __UDS_STREAM_HPP__
#define __UDS_STREAM_HPP__

#include "stream/stream_listener.hpp"
#include <string>

/* 将流数据通过 Unix Domain Socket 发给客户端 */
class uds_stream : public stream_listener {
public:
    explicit uds_stream(int client_fd = -1);
    ~uds_stream() override;
    void set_client_fd(int fd) { fd_ = fd; }
    void on_stream_data(const char *data, int len) override;

private:
    int fd_;
};

#endif /* __UDS_STREAM_HPP__ */
