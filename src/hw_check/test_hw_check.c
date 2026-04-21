/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * @author: LiJun
 * @date: 2026-04-21
 * @fileName: test_hw_check.c
 * @description: hw_check 各平台统一测试入口
 *
 * 宏控选择测试平台：
 *   -DHW_CHECK_TYPE=0  -> 测试 libhw_check_hdmi.so
 *   -DHW_CHECK_TYPE=1  -> 测试 libhw_check_usb.so
 *
 * 编译：
 *   cd hw_check/build && \
 *   gcc -o test_hw_check_hdmi ../test_hw_check.c -DHW_CHECK_TYPE=0 \
 *       -I.. -I../.. -I../../log -ldl
 *
 * 运行：
 *   ./test_hw_check_hdmi ./libhw_check_hdmi.so
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdbool.h>

#include "hw_check_intf.h"

/* ============================================================================
 * 平台宏检测
 * ============================================================================ */
#if HW_CHECK_TYPE == 0
    #define HW_CHECK_PLATFORM_NAME "hdmi"
    #define HW_CHECK_SO_NAME       "libhw_check_hdmi.so"
#elif HW_CHECK_TYPE == 1
    #define HW_CHECK_PLATFORM_NAME "usb"
    #define HW_CHECK_SO_NAME       "libhw_check_usb.so"
#else
    #error "请定义 -DHW_CHECK_TYPE=0(hdmi) 或 -DHW_CHECK_TYPE=1(usb)"
#endif

#define HW_CHECK_CREATE_SYM  HW_CHECK_CREATE_SYMBOL

/* ============================================================================
 * 辅助宏
 * ============================================================================ */
#define TEST_ASSERT_NOT_NULL(ptr, msg) do { \
    if ((ptr) == NULL) { \
        fprintf(stderr, "[FAIL] %s: %s is NULL\n", __func__, msg); \
        exit(1); \
    } \
} while (0)

#define TEST_ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "[FAIL] %s: %s (expected %d, got %d)\n", \
                __func__, msg, (int)(b), (int)(a)); \
        exit(1); \
    } \
} while (0)

#define TEST_PASS() printf("[PASS] %s\n", __func__)

/* ============================================================================
 * 测试1: dlopen + dlsym hw_check_intf_create
 * ============================================================================ */
static void test_dlopen_symbol(const char *so_path)
{
    void *handle = dlopen(so_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "[FAIL] dlopen(%s) failed: %s\n", so_path, dlerror());
        exit(1);
    }
    printf("[INFO] dlopen %s OK\n", so_path);

    (void)dlerror();
    hw_check_create_fn create_fn = (hw_check_create_fn)dlsym(handle, HW_CHECK_CREATE_SYM);
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "[FAIL] dlsym(%s) failed: %s\n", HW_CHECK_CREATE_SYM, err);
        dlclose(handle);
        exit(1);
    }

    TEST_ASSERT_NOT_NULL(create_fn, HW_CHECK_CREATE_SYM);
    TEST_PASS();

    (void)handle; /* 保持 handle 打开 */
}

/* ============================================================================
 * 测试2: 验证 hw_check_intf 结构体
 * ============================================================================ */
static void test_intf_struct(void *handle)
{
    hw_check_create_fn create_fn = (hw_check_create_fn)dlsym(handle, HW_CHECK_CREATE_SYM);
    struct hw_check_intf *intf = create_fn();

    TEST_ASSERT_NOT_NULL(intf, "hw_check_intf");
    TEST_ASSERT_NOT_NULL(intf->name, "intf->name");
    TEST_ASSERT_NOT_NULL(intf->opt, "intf->opt");
    TEST_ASSERT_NOT_NULL(intf->opt->create, "opt->create");
    TEST_ASSERT_NOT_NULL(intf->opt->destroy, "opt->destroy");
    TEST_ASSERT_NOT_NULL(intf->opt->get_exist, "opt->get_exist");

    printf("[INFO] platform name: %s\n", intf->name);
    TEST_PASS();
}

/* ============================================================================
 * 测试3: create / get_exist / destroy 路径
 * ============================================================================ */
static void test_create_get_exist_destroy(void *handle)
{
    hw_check_create_fn create_fn = (hw_check_create_fn)dlsym(handle, HW_CHECK_CREATE_SYM);
    struct hw_check_intf *intf = create_fn();

    /* 创建实例 */
    struct hw_check_ctx *ctx = intf->opt->create(HW_TYPE_HDMI, NULL, NULL, false);
    TEST_ASSERT_NOT_NULL(ctx, "create()");

    /* get_exist 多次调用 */
    bool exist1 = intf->opt->get_exist(ctx);
    printf("[INFO] get_exist() -> %d\n", exist1);

    bool exist2 = intf->opt->get_exist(ctx);
    printf("[INFO] get_exist() again -> %d\n", exist2);

    /* 销毁实例 */
    intf->opt->destroy(ctx);
    TEST_PASS();
}

/* ============================================================================
 * 测试4: 线程检测模式（run_thread_check=true）
 * ============================================================================ */
static void test_thread_mode(void *handle)
{
    hw_check_create_fn create_fn = (hw_check_create_fn)dlsym(handle, HW_CHECK_CREATE_SYM);
    struct hw_check_intf *intf = create_fn();

    struct hw_check_ctx *ctx = intf->opt->create(HW_TYPE_HDMI, NULL, NULL, true);
    TEST_ASSERT_NOT_NULL(ctx, "create(thread_mode)");

    /* 等待 2 秒，观察线程是否在运行 */
    sleep(2);

    /* get_exist 应该返回当前状态 */
    bool exist = intf->opt->get_exist(ctx);
    printf("[INFO] thread mode get_exist() -> %d\n", exist);

    intf->opt->destroy(ctx);
    TEST_PASS();
}

/* ============================================================================
 * main
 * ============================================================================ */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hw_check.so>\n", argv[0]);
        fprintf(stderr, "Example: %s ./libhw_check_hdmi.so\n", argv[0]);
        return 1;
    }

    const char *so_path = argv[1];

    printf("====================================\n");
    printf("  hw_check (%s) 单元测试\n", HW_CHECK_PLATFORM_NAME);
    printf("====================================\n\n");

    /* 先 dlopen 保持 handle 存活，后续测试共享同一个 handle */
    void *handle = dlopen(so_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "[FAIL] dlopen(%s) failed: %s\n", so_path, dlerror());
        return 1;
    }

    printf("[TEST] test_dlopen_symbol\n");
    test_dlopen_symbol(so_path);
    printf("\n");

    printf("[TEST] test_intf_struct\n");
    test_intf_struct(handle);
    printf("\n");

    printf("[TEST] test_create_get_exist_destroy\n");
    test_create_get_exist_destroy(handle);
    printf("\n");

    printf("[TEST] test_thread_mode\n");
    test_thread_mode(handle);
    printf("\n");

    dlclose(handle);

    printf("====================================\n");
    printf("  全部测试通过\n");
    printf("====================================\n");
    return 0;
}
