#include "uds_stream.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

uds_stream::uds_stream(int client_fd)
    : fd_(client_fd)
{
}

uds_stream::~uds_stream()
{
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

void uds_stream::on_data_input(const char *data, int len)
{
    if (fd_ < 0 || len <= 0)
        return;
    size_t sent = 0;
    while (sent < (size_t)len) {
        ssize_t n = send(fd_, data + sent, len - sent, 0);
        if (n <= 0)
            break;
        sent += n;
    }
}
