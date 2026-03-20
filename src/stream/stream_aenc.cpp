#include "stream_pipe.hpp"
#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"
#include <cstring>
#include <unistd.h>

stream_aenc::stream_aenc(int chn)
    : chn_(chn)
{
}

void stream_aenc::stream_data_loop()
{
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (!mpi || !mpi->audio || !mpi->audio->aenc_get_data || !mpi->ctx_data)
        return;

    char buf[256 * 1024];
    int len;

    while (is_running()) {
        len = sizeof(buf);
        if (mpi->audio->aenc_get_data(&mpi->ctx_data->a_ctx.aenc[chn_], buf, &len) == 0 && len > 0)
            notify_listeners(buf, len);
        else
            usleep(5000);
    }
}
