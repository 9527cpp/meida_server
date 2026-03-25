#include "stream_base.hpp"
#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"
#include <cstring>
#include <unistd.h>

#define MODULE_TAG "stream_venc"

#include "log/log_tag.h"

stream_venc::stream_venc(int chn)
    : chn_(chn)
{
}

void stream_venc::stream_data_loop()
{
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (!mpi || !mpi->video || !mpi->video->venc_get_data || !mpi->ctx_data)
        return;

    char buf[256 * 1024];
    int len;

    while (is_running()) {
        // WriteLog(LOG_DEBUG, "stream_data_loop: %p", this);
        len = sizeof(buf);
        if (mpi->video->venc_get_data(&mpi->ctx_data->v_ctx.venc[chn_], buf, &len) == 0 && len > 0)
            notify_listeners(buf, len);
        else
            usleep(5000);
    }
}
