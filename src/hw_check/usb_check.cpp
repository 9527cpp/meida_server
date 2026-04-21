#include "hw_check.hpp"
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <regex>

namespace {

bool is_usb_device(int fd)
{
    struct v4l2_capability cap{};
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
        return false;

    // Check if device has video capture capability
    if (cap.capabilities & V4L2_CAP_DEVICE_CAPS) {
        if (!(cap.device_caps & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE)))
            return false;
    } else {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
            return false;
    }

    // Check bus_info for "usb"
    std::string bus_info(reinterpret_cast<char*>(cap.bus_info));
    if (bus_info.find("usb") != std::string::npos)
        return true;

    return false;
}

bool check_usb_video_device(const char* dev_name)
{
    std::string dev_path = "/dev/" + std::string(dev_name);

    int fd = open(dev_path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0)
        return false;

    bool is_usb = is_usb_device(fd);
    close(fd);
    return is_usb;
}

}  // namespace

usb_check::usb_check(hw_check_listener *listener, bool run_thread_check)
    : hw_check(hw_type::usb, listener, run_thread_check)
{
    check_once();
}

void usb_check::check_once()
{
    bool found = false;

    DIR* dir = opendir("/dev");
    if (!dir)
        goto out;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strncmp(entry->d_name, "video", 5) != 0)
            continue;

        if (check_usb_video_device(entry->d_name)) {
            found = true;
            break;
        }
    }
    closedir(dir);

out:
    set_hw_exist(found);
}