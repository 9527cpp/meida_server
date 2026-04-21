#include "hw_check.hpp"
#include <fstream>
#include <cstring>

hdmi_check::hdmi_check(hw_check_listener *listener, bool run_thread_check)
    : hw_check(hw_type::hdmi, listener, run_thread_check)
{
}

void hdmi_check::check_once()
{
    std::ifstream f("/tmp/hdinfo");
    if (!f.is_open()) {
        set_hw_exist(false);
        return;
    }

    char buffer[256];
    f.getline(buffer, sizeof(buffer));
    // format: "chipver=0;width=1920;height=1080;time=1609303283"
    const char* width_str = strstr(buffer, "width=");
    const char* height_str = strstr(buffer, "height=");
    if (!width_str || !height_str) {
        set_hw_exist(false);
        return;
    }

    int width = atoi(width_str + strlen("width="));
    int height = atoi(height_str + strlen("height="));
    set_hw_exist(width > 0 && height > 0);
}
