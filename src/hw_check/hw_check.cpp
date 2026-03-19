#include "hw_check.hpp"
#include <unistd.h>

hw_check::hw_check(bool run_thread_check)
    : is_hw_exist_(false)
    , run_thread_(false)
{
    if (run_thread_check) {
        run_thread_ = true;
        th_ = std::thread(&hw_check::check_loop, this);
    }
}

hw_check::~hw_check()
{
    run_thread_ = false;
    if (th_.joinable())
        th_.join();
}

void hw_check::check_loop()
{
    while (run_thread_) {
        check_once();
        sleep(1);
    }
}
