/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * @author: LiJun
 * @date: 2026-04-21
 * @fileName: demo_hw_check.c
 * @description: hw_check 各平台全流程演示
 *
 * 演示 hw_check .so 从加载到检测到释放的完整生命周期：
 *   1. dlopen .so，dlsym hw_check_intf_create
 *   2. 创建 hw_check 实例
 *   3. 轮询 get_exist 检测硬件状态
 *   4. 销毁实例并释放 .so
 *
 * 用法：
 *   ./demo_hw_check -w ./libhw_check_hdmi.so
 *
 * 宏控选择平台：
 *   -DHW_CHECK_TYPE=0  -> hdmi
 *   -DHW_CHECK_TYPE=1  -> usb
 *
 * 编译：
 *   cd hw_check/build && \
 *   gcc -o demo_hw_check_hdmi ../demo_hw_check.c -DHW_CHECK_TYPE=0 \
 *       -I.. -I../../log -lpthread -ldl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <getopt.h>
#include <stdbool.h>

#include "hw_check_intf.h"

/* ============================================================================
 * 平台宏检测
 * ============================================================================ */
#if HW_CHECK_TYPE == 0
    #define HW_CHECK_PLATFORM_NAME "hdmi"
#elif HW_CHECK_TYPE == 1
    #define HW_CHECK_PLATFORM_NAME "usb"
#else
    #error "请定义 -DHW_CHECK_TYPE=0(hdmi) 或 -DHW_CHECK_TYPE=1(usb)"
#endif

#define HW_CHECK_CREATE_SYM  HW_CHECK_CREATE_SYMBOL

/* ============================================================================
 * 命令行参数
 * ============================================================================ */
static const char *g_so_path = NULL;

static void usage(const char *prog)
{
    fprintf(stderr, "用法: %s -w <hw_check.so>\n", prog);
    fprintf(stderr, "示例: %s -w ./libhw_check_%s.so\n", prog,
#if HW_CHECK_TYPE == 0
            "hdmi"
#else
            "usb"
#endif
    );
    exit(1);
}

static int parse_args(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "w:h")) != -1) {
        switch (opt) {
        case 'w':
            g_so_path = optarg;
            break;
        case 'h':
        default:
            usage(argv[0]);
            break;
        }
    }
    if (!g_so_path) {
        fprintf(stderr, "错误: 必须指定 -w 参数\n");
        usage(argv[0]);
    }
    return 0;
}

/* ============================================================================
 * 辅助函数
 * ============================================================================ */
static void *load_so(const char *path)
{
    void *h = dlopen(path, RTLD_NOW);
    if (h) {
        printf("[DEMO] dlopen %s OK\n", path);
        return h;
    }
    fprintf(stderr, "[DEMO] dlopen %s failed: %s\n", path, dlerror());
    return NULL;
}

static hw_check_create_fn get_create_fn(void *handle)
{
    (void)dlerror();
    hw_check_create_fn fn = (hw_check_create_fn)dlsym(handle, HW_CHECK_CREATE_SYM);
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "[DEMO] dlsym(%s) failed: %s\n", HW_CHECK_CREATE_SYM, err);
        return NULL;
    }
    printf("[DEMO] dlsym(%s) OK\n", HW_CHECK_CREATE_SYM);
    return fn;
}

/* ============================================================================
 * 演示：轮询检测硬件状态
 * ============================================================================ */
static void demo_poll_hw_status(struct hw_check_intf *intf)
{
    printf("\n========== Demo Poll HW Status ==========\n");

    struct hw_check_ctx *ctx = intf->opt->create(HW_TYPE_HDMI, NULL, NULL, true);
    if (!ctx) {
        fprintf(stderr, "[DEMO] create() returned NULL\n");
        return;
    }
    printf("[DEMO] hw_check instance created (thread mode)\n");

    /* 轮询 5 次，每次间隔 1 秒 */
    for (int i = 0; i < 5; i++) {
        bool exist = intf->opt->get_exist(ctx);
        printf("[DEMO] [%d] hw exist: %s\n", i, exist ? "yes" : "no");
        sleep(1);
    }

    intf->opt->destroy(ctx);
    printf("[DEMO] hw_check instance destroyed\n");
    printf("[DEMO] Poll HW Status demo done\n");
}

/* ============================================================================
 * 演示：单次检测（无线程模式）
 * ============================================================================ */
static void demo_single_check(struct hw_check_intf *intf)
{
    printf("\n========== Demo Single Check ==========\n");

    struct hw_check_ctx *ctx = intf->opt->create(HW_TYPE_HDMI, NULL, NULL, false);
    if (!ctx) {
        fprintf(stderr, "[DEMO] create() returned NULL\n");
        return;
    }
    printf("[DEMO] hw_check instance created (single check mode)\n");

    bool exist = intf->opt->get_exist(ctx);
    printf("[DEMO] hw exist: %s\n", exist ? "yes" : "no");

    intf->opt->destroy(ctx);
    printf("[DEMO] hw_check instance destroyed\n");
    printf("[DEMO] Single Check demo done\n");
}

/* ============================================================================
 * main
 * ============================================================================ */
int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    printf("========================================\n");
    printf("  hw_check (%s) 全流程演示\n", HW_CHECK_PLATFORM_NAME);
    printf("========================================\n");

    /* 1. 加载 .so */
    void *handle = load_so(g_so_path);
    if (!handle) {
        return 1;
    }

    /* 2. 获取 create 函数 */
    hw_check_create_fn create_fn = get_create_fn(handle);
    if (!create_fn) {
        dlclose(handle);
        return 1;
    }

    /* 3. 创建接口实例 */
    struct hw_check_intf *intf = create_fn();
    if (!intf) {
        fprintf(stderr, "[DEMO] hw_check_intf_create() returned NULL\n");
        dlclose(handle);
        return 1;
    }
    printf("[DEMO] hw_check_intf_create() OK, platform: %s\n", intf->name);

    /* 4. 演示 */
    demo_single_check(intf);
    demo_poll_hw_status(intf);

    printf("\n========================================\n");
    printf("  Demo 全部完成\n");
    printf("========================================\n");

    dlclose(handle);
    return 0;
}
