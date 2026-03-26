#include "file_stream.hpp"
#include <cstring>

file_stream::file_stream(const std::string &path)
    : path_(path)
    , fp_(nullptr)
{
    fp_ = fopen(path_.c_str(), "wb");
}

file_stream::~file_stream()
{
    if (fp_) {
        fclose(fp_);
        fp_ = nullptr;
    }
}

void file_stream::on_data_input(const char *data, int len)
{
    if (!fp_ || len <= 0)
        return;
    fwrite(data, 1, len, fp_);
    fflush(fp_);
}
