/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Oray Inc.
 *
 * @author: LiJun
 * @date: 2026-03-20
 * @fileName: hw_check.hpp
 * @description: 硬件检测基类, 具体实现检测的再子类中实现, 该类只负责检测框架并通知监听器, 后续可再进行包装添加uds功能
 */

#ifndef __HW_CHECK_HPP__
#define __HW_CHECK_HPP__

#include <atomic>
#include <thread>


/* 硬件类型 */
enum class hw_type {
    hdmi = 0,
    usb = 1,
    mic = 2,
};

/* 硬件事件 */
enum class hw_event {
    plug_in = 0,
    plug_out = 1,
};

/* 硬件检测监听器：当检测到插入/移除时回调 */
class hw_check_listener {
public:
    virtual ~hw_check_listener() = default;
    virtual void on_hw_check_notify(hw_type type, hw_event ev) = 0;
};

/* 硬件检测：启动时检测一次，可选线程轮询；后续可接入 ubus 通知更新 */
class hw_check {
public:
    explicit hw_check(hw_type type,
        hw_check_listener *listener = nullptr,
        bool run_thread_check = false);
    virtual ~hw_check();
    bool get_hw_exist() const { return is_hw_exist_.load(); }

protected:
    /* 子类实现具体检测逻辑；基类构造中不调用，由子类构造末尾调用 */
    virtual void check_once() {}

    /* 由子类调用：更新检测结果并在状态变化时通知监听器 */
    void set_hw_exist(bool exist);

private:
    void check_loop();

    hw_type type_;
    hw_check_listener *listener_;
    std::atomic<bool> is_hw_exist_;
    std::thread th_;
    std::atomic<bool> run_thread_;
};

/* 检测 HDMI：读取 /tmp/hdinfo 等判断是否有分辨率信息 */
class hdmi_check : public hw_check {
public:
    explicit hdmi_check(hw_check_listener *listener = nullptr, bool run_thread_check = false);
protected:
    void check_once() override;
};

/* 检测 USB 摄像头：枚举 USB video 设备 */
class usb_check : public hw_check {
public:
    explicit usb_check(hw_check_listener *listener = nullptr, bool run_thread_check = false);
protected:
    void check_once() override;
};


#endif /* __HW_CHECK_HPP__ */
