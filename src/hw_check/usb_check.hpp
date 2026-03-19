#ifndef __USB_CHECK_HPP__
#define __USB_CHECK_HPP__

#include "hw_check.hpp"

/* 检测 USB 摄像头：枚举 USB video 设备 */
class usb_check : public hw_check {
public:
    explicit usb_check(bool run_thread_check = false);
protected:
    void check_once() override;
};

#endif /* __USB_CHECK_HPP__ */
