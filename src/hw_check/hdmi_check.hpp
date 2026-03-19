#ifndef __HDMI_CHECK_HPP__
#define __HDMI_CHECK_HPP__

#include "hw_check.hpp"

/* 检测 HDMI：读取 /tmp/hdinfo 等判断是否有分辨率信息 */
class hdmi_check : public hw_check {
public:
    explicit hdmi_check(bool run_thread_check = false);
protected:
    void check_once() override;
};

#endif /* __HDMI_CHECK_HPP__ */
