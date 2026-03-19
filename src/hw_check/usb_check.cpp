#include "usb_check.hpp"
#include <dirent.h>
#include <cstring>

usb_check::usb_check(bool run_thread_check)
    : hw_check(run_thread_check)
{
    check_once();
}

void usb_check::check_once()
{
    bool found = false;
    DIR *d = opendir("/dev");
    if (!d)
        goto out;
    struct dirent *e;
    while ((e = readdir(d)) != nullptr) {
        if (strncmp(e->d_name, "video", 5) == 0) {
            found = true;
            break;
        }
    }
    closedir(d);
out:
    set_hw_exist(found);
}
