/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * @author: LiJun
 * @date: 2026-04-21
 * @fileName: hw_check_intf.h
 * @description: hw_check 插件 C 接口定义
 *
 * 各平台 .so 须导出 hw_check_intf_create 符号，返回 struct hw_check_intf*。
 * main 通过 dlopen/dlsym(HW_CHECK_CREATE_SYMBOL) 获取此函数指针后调用。
 */

#ifndef __HW_CHECK_INTF_H__
#define __HW_CHECK_INTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* 硬件类型 */
enum hw_check_type {
    HW_TYPE_HDMI = 0,
    HW_TYPE_USB  = 1,
    HW_TYPE_MIC  = 2,
};

/* 硬件事件 */
enum hw_check_event {
    HW_EV_PLUG_IN  = 0,
    HW_EV_PLUG_OUT = 1,
};

/* 硬件检测回调（由调用者提供，.so 内部在检测到变化时调用） */
typedef void (*hw_check_notify_fn)(enum hw_check_type type,
                                    enum hw_check_event ev,
                                    void *user_data);

/* hw_check 实例上下文（不透明，由实现者管理） */
struct hw_check_ctx;

/* hw_check 操作接口 */
struct hw_check_opt {
    /* 创建检测实例
     * type: 检测类型（HDMI/USB/MIC）
     * notify: 状态变化回调（可为 NULL）
     * user_data: 回调用户数据
     * run_thread_check: 是否启动线程轮询
     * 返回 ctx 句柄，失败返回 NULL */
    struct hw_check_ctx *(*create)(enum hw_check_type type,
                                   hw_check_notify_fn notify,
                                   void *user_data,
                                   bool run_thread_check);

    /* 销毁检测实例 */
    void (*destroy)(struct hw_check_ctx *ctx);

    /* 获取当前硬件是否存在 */
    bool (*get_exist)(struct hw_check_ctx *ctx);
};

/* hw_check 接口实例（每个 .so 只导出唯一一个） */
struct hw_check_intf {
    const char *name;       /* e.g. "hdmi", "usb" */
    struct hw_check_opt *opt;
};

/*
 * 动态库导出约定：每个平台 .so 须导出此符号，返回该平台的 hw_check_intf 实例。
 * main 通过 dlsym(handle, HW_CHECK_CREATE_SYMBOL) 获取此函数指针后调用。
 */
#define HW_CHECK_CREATE_SYMBOL "hw_check_intf_create"
typedef struct hw_check_intf *(*hw_check_create_fn)(void);

#ifdef __cplusplus
}
#endif

#endif /* __HW_CHECK_INTF_H__ */
