/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * @author: LiJun
 * @date: 2026-04-21
 * @fileName: hw_check_factory.cpp
 * @description: hw_check .so 导出符号实现（hdmi 版本）
 *
 * 编译到 libhw_check_hdmi.so 中，仅包含 hdmi 检测实现。
 */

#include "hw_check_intf.h"
#include "hw_check.hpp"

static const int MAX_HW_CHECK_INSTANCES = 16;

struct hw_check_ctx_impl {
    hw_check *obj;
    hw_check_notify_fn notify;
    void *user_data;
};

static hw_check_notify_fn g_notify_fn[MAX_HW_CHECK_INSTANCES];
static void *g_user_data[MAX_HW_CHECK_INSTANCES];
static int g_instance_count = 0;

static struct hw_check_ctx *hdmi_create(enum hw_check_type type,
                                        hw_check_notify_fn notify,
                                        void *user_data,
                                        bool run_thread_check)
{
    (void)type;
    if (g_instance_count >= MAX_HW_CHECK_INSTANCES)
        return NULL;

    int idx = g_instance_count++;
    g_notify_fn[idx] = notify;
    g_user_data[idx] = user_data;

    auto *impl = new hw_check_ctx_impl;
    impl->notify = notify;
    impl->user_data = user_data;
    impl->obj = new hdmi_check(nullptr, run_thread_check);

    return reinterpret_cast<struct hw_check_ctx*>(impl);
}

static void hdmi_destroy(struct hw_check_ctx *ctx)
{
    if (!ctx) return;
    auto *impl = reinterpret_cast<hw_check_ctx_impl*>(ctx);
    delete impl->obj;
    delete impl;
}

static bool hdmi_get_exist(struct hw_check_ctx *ctx)
{
    if (!ctx) return false;
    auto *impl = reinterpret_cast<hw_check_ctx_impl*>(ctx);
    return impl->obj->get_hw_exist();
}

static struct hw_check_opt hdmi_opt = {
    .create    = hdmi_create,
    .destroy   = hdmi_destroy,
    .get_exist = hdmi_get_exist,
};

static struct hw_check_intf hdmi_intf = {
    .name = "hdmi",
    .opt  = &hdmi_opt,
};

extern "C" {

struct hw_check_intf *hw_check_intf_create(void)
{
    return &hdmi_intf;
}

}
