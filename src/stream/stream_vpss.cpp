#include "stream_vpss.hpp"
#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"
#include <cstring>
#include <unistd.h>

#define MODULE_TAG "stream_vpss"

#include "log/log_tag.h"

stream_vpss::stream_vpss(int chn)
    : chn_(chn)
{
}

void stream_vpss::stream_data_loop()
{
    struct mpi_intf *mpi = mpi_intf_get_instance();
    if (!mpi || !mpi->video || !mpi->video->vpss_get_data || !mpi->ctx_data)
        return;

    char buf[256 * 1024];
    int len;

    while (is_running()) {
        len = sizeof(buf);
        if (mpi->video->vpss_get_data(&mpi->ctx_data->v_ctx.vpss[chn_], buf, &len) == 0 && len > 0)
            on_data_input(buf, len);
        else
            usleep(5000);
    }
}
