#ifndef __HW_CHECK_HPP__
#define __HW_CHECK_HPP__

#include <atomic>
#include <thread>

/* 硬件检测：启动时检测一次，可选线程轮询；后续可接入 ubus 通知更新 */
class hw_check {
public:
    explicit hw_check(bool run_thread_check = false);
    virtual ~hw_check();

    void set_hw_exist(bool exist) { is_hw_exist_.store(exist); }
    bool get_hw_exist() const { return is_hw_exist_.load(); }

protected:
    /* 子类实现具体检测逻辑；基类构造中不调用，由子类构造末尾调用 */
    virtual void check_once() {}

private:
    void check_loop();

    std::atomic<bool> is_hw_exist_;
    std::thread th_;
    std::atomic<bool> run_thread_;
};

#endif /* __HW_CHECK_HPP__ */
