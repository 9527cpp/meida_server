#include "hw_check.hpp"
#include <unistd.h>

hw_check::hw_check(hw_type type, hw_check_listener *listener, bool run_thread_check)
    : type_(type)
    , listener_(listener)
    , is_hw_exist_(false)
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

// check_once 掉用 set_hw_exist, 只有发生变化时才通知监听器
void hw_check::set_hw_exist(bool exist)
{
    bool old = is_hw_exist_.exchange(exist);
    if (old == exist)
        return;

    hw_check_listener *l = listener_;
    if (!l)
        return;

    hw_event ev = exist ? hw_event::plug_in : hw_event::plug_out;
    l->on_hw_check_notify(type_, ev);
}

void hw_check::check_loop()
{
    while (run_thread_) {
        check_once();
        sleep(1);
    }
}
