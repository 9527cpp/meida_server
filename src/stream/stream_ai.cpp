#include "stream_base.hpp"
#include <unistd.h>

stream_ai::stream_ai()
{
}

void stream_ai::stream_data_loop()
{
    while (is_running())
        usleep(100000);
}

