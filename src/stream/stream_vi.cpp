#include "stream_pipe.hpp"
#include <unistd.h>
#define MODULE_TAG "stream_vi"

#include "log/log_tag.h"

stream_vi::stream_vi()
{
}

void stream_vi::stream_data_loop()
{
    /* 若平台提供 vi_get_data 可在此取数并 notify_listeners；当前由 VENC 直接取数 */
    while (is_running()) {
        WriteLog(LOG_DEBUG, "stream_data_loop: %p", this);
        usleep(100000);
    }
}
