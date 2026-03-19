#include "hdmi_check.hpp"
#include <fstream>
#include <string>

hdmi_check::hdmi_check(bool run_thread_check)
    : hw_check(run_thread_check)
{
    check_once();
}

void hdmi_check::check_once()
{
    std::ifstream f("/tmp/hdinfo");
    if (!f.is_open()) {
        set_hw_exist(false);
        return;
    }
    std::string line;
    bool has_res = false;
    while (std::getline(f, line)) {
        if (line.find("resolution") != std::string::npos ||
            line.find("x") != std::string::npos) {
            has_res = true;
            break;
        }
    }
    set_hw_exist(has_res);
}
