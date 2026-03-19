#ifndef __FILE_STREAM_HPP__
#define __FILE_STREAM_HPP__

#include "../stream/stream_listener.hpp"
#include <string>
#include <cstdio>

/* 将流数据写入文件保存 */
class file_stream : public stream_listener {
public:
    explicit file_stream(const std::string &path);
    ~file_stream() override;
    void on_stream_data(const char *data, int len) override;

private:
    std::string path_;
    FILE *fp_;
};

#endif /* __FILE_STREAM_HPP__ */
