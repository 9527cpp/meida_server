#include "stream_venc.hpp"
#include "../mpi/mpi_intf.h"
#include "../mpi_ctx/mpi_ctx_intf.h"
#include <cstring>
#include <unistd.h>

stream_venc::stream_venc(int chn)
    : chn_(chn)
{
}

void stream_venc::on_stream_data(const char *data, int len)
{
    /* 作为 pipe 中段时：收到上一级数据可在此做处理后再转发，当前直接转发 */
    notify_listeners(data, len);
}

void stream_venc::stream_data_loop()
{
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (!mpi || !mpi->video || !mpi->video->venc_get_data || !mpi->ctx_data)
        return;

    char buf[256 * 1024];
    int len;

    while (is_running()) {
        len = sizeof(buf);
        if (mpi->video->venc_get_data(&mpi->ctx_data->v_ctx.venc[chn_], buf, &len) == 0 && len > 0)
            notify_listeners(buf, len);
        else
            usleep(5000);
    }
}
