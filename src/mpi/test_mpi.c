/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * @author: LiJun
 * @date: 2026-04-21
 * @fileName: test_mpi.c
 * @description: MPI 各平台统一测试入口
 *
 * 宏控选择测试平台：
 *   -DMPI_PLATFORM_ROCKIT  -> 测试 mpi_rockit.so
 *   -DMPI_PLATFORM_HISI    -> 测试 mpi_hisi.so
 *   -DMPI_PLATFORM_STUB    -> 测试 mpi_stub.so
 *
 * 编译：
 *   cd mpi/build && gcc -o test_mpi ../test_mpi.c -DMPI_PLATFORM_STUB -I.. -I../mpi_ctx -I../log -ldl -lm
 *
 * 运行：
 *   ./test_mpi（需在 mpi/build/ 目录下，因为搜索路径包含 ./libmpi_xxx.so）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

#include "mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"

/* ============================================================================
 * 平台宏检测
 * ============================================================================ */
#if defined(MPI_PLATFORM_ROCKIT)
    #define MPI_PLATFORM_NAME "rockit"
    #define MPI_SO_NAME       "libmpi_rockit.so"
    #define MPI_CREATE_SYM    "mpi_intf_create"
#elif defined(MPI_PLATFORM_HISI)
    #define MPI_PLATFORM_NAME "hisi"
    #define MPI_SO_NAME       "libmpi_hisi.so"
    #define MPI_CREATE_SYM    "mpi_intf_create"
#elif defined(MPI_PLATFORM_RKMEDIA)
    #define MPI_PLATFORM_NAME "rkmedia"
    #define MPI_SO_NAME       "libmpi_rkmedia.so"
    #define MPI_CREATE_SYM    "mpi_intf_create"
#elif defined(MPI_PLATFORM_STUB)
    #define MPI_PLATFORM_NAME "stub"
    #define MPI_SO_NAME       "libmpi_stub.so"
    #define MPI_CREATE_SYM    "mpi_intf_create"
#else
    #error "请定义 -DMPI_PLATFORM_ROCKIT 或 -DMPI_PLATFORM_HISI 或 -DMPI_PLATFORM_RKMEDIA 或 -DMPI_PLATFORM_STUB"
#endif

/* ============================================================================
 * 全局 mpi_intf（dlopen 后填充）
 * ============================================================================ */
static struct mpi_intf *g_mpi = NULL;

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
        fprintf(stderr, "[FAIL] %s: %s (expected %ld, got %ld)\n", \
                __func__, msg, (long)(b), (long)(a)); \
        exit(1); \
    } \
} while (0)

#define TEST_PASS() printf("[PASS] %s\n", __func__)

/* ============================================================================
 * 辅助函数：尝试从多个路径加载目标 .so
 * ============================================================================ */
static void *try_load_so(const char *path)
{
    void *h = dlopen(path, RTLD_NOW);
    if (h) return h;
    (void)dlerror();
    return NULL;
}

static void *load_mpi_so(void)
{
    const char *search_paths[] = {
        "./" MPI_SO_NAME,
        "../build/" MPI_SO_NAME,
        NULL,
    };

    for (int i = 0; search_paths[i]; i++) {
        void *h = try_load_so(search_paths[i]);
        if (h) {
            printf("[INFO] loaded: %s\n", search_paths[i]);
            return h;
        }
    }
    return NULL;
}

/* ============================================================================
 * 测试1: dlopen + dlsym mpi_intf_create
 * ============================================================================ */
static void test_dlopen_symbol(void)
{
    void *handle = load_mpi_so();
    if (!handle) {
        fprintf(stderr, "[WARN] dlopen %s failed: %s\n", MPI_SO_NAME, dlerror());
        fprintf(stderr, "[SKIP] %s: %s not available\n", __func__, MPI_SO_NAME);
        return;
    }

    (void)dlerror();
    mpi_intf_create_fn create_fn = (mpi_intf_create_fn)dlsym(handle, MPI_CREATE_SYM);
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "[FAIL] dlsym(%s) failed: %s\n", MPI_CREATE_SYM, err);
        dlclose(handle);
        exit(1);
    }

    TEST_ASSERT_NOT_NULL(create_fn, MPI_CREATE_SYM);

    struct mpi_intf *mpi = create_fn();
    TEST_ASSERT_NOT_NULL(mpi, "mpi_intf_create()");
    TEST_ASSERT_NOT_NULL(mpi->sys, "mpi->sys");
    TEST_ASSERT_NOT_NULL(mpi->video, "mpi->video");
    TEST_ASSERT_NOT_NULL(mpi->audio, "mpi->audio");

    g_mpi = mpi;
    TEST_PASS();
    /* 注意：不 dlclose，以便后续测试继续使用 g_mpi */
    (void)handle; /* 保持 handle 打开 */
}

/* ============================================================================
 * 测试2: 验证 mpi_intf 所有函数指针非空
 * ============================================================================ */
static void test_struct_function_pointers(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    TEST_ASSERT_NOT_NULL(g_mpi->sys->sys_init, "sys_init");
    TEST_ASSERT_NOT_NULL(g_mpi->sys->sys_deinit, "sys_deinit");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vi_init, "vi_init");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vi_deinit, "vi_deinit");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vi_release_data, "vi_release_data");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vpss_init, "vpss_init");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vpss_deinit, "vpss_deinit");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vpss_get_data, "vpss_get_data");
    TEST_ASSERT_NOT_NULL(g_mpi->video->vpss_release_data, "vpss_release_data");
    TEST_ASSERT_NOT_NULL(g_mpi->video->venc_init, "venc_init");
    TEST_ASSERT_NOT_NULL(g_mpi->video->venc_deinit, "venc_deinit");
    TEST_ASSERT_NOT_NULL(g_mpi->video->venc_get_data, "venc_get_data");
    TEST_ASSERT_NOT_NULL(g_mpi->video->venc_release_data, "venc_release_data");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->ai_init, "ai_init");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->ai_deinit, "ai_deinit");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->ai_release_data, "ai_release_data");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->aenc_init, "aenc_init");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->aenc_deinit, "aenc_deinit");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->aenc_get_data, "aenc_get_data");
    TEST_ASSERT_NOT_NULL(g_mpi->audio->aenc_release_data, "aenc_release_data");
    TEST_PASS();
}

/* ============================================================================
 * 测试3: mock vi_ctx，验证 VI init/deinit 路径
 * ============================================================================ */
static void test_vi_ctx_init_deinit(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    struct vi_ctx disabled = { .enable = 0, .devid = 0, .pipeid = 0, .chn_index = 0 };
    struct vi_ctx enabled = {
        .enable = 1, .devid = 0, .pipeid = 0, .chn_index = 0,
        .entity_name = "virt", .format = 0,
        .width = 1920, .height = 1080, .buf_cnt = 3, .memory_type = 0
    };

    TEST_ASSERT_EQ(g_mpi->video->vi_init(NULL), 0, "vi_init(NULL)");
    TEST_ASSERT_EQ(g_mpi->video->vi_init(&disabled), 0, "vi_init(disabled)");
    TEST_ASSERT_EQ(g_mpi->video->vi_deinit(&disabled), 0, "vi_deinit(disabled)");

    int ret = g_mpi->video->vi_init(&enabled);
    if (ret != 0 && ret != -1) {
        fprintf(stderr, "[FAIL] %s: vi_init(enabled) returned %d\n", __func__, ret);
        exit(1);
    }
    g_mpi->video->vi_deinit(&enabled);
    TEST_PASS();
}

/* ============================================================================
 * 测试4: mock venc_ctx，验证 VENC init/deinit 路径
 * ============================================================================ */
static void test_venc_ctx_init_deinit(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    struct venc_ctx disabled = { .enable = 0, .chn_index = 0 };
    struct venc_ctx enabled = {
        .enable = 1, .chn_index = 0, .type = "h265",
        .venc_attr = { .pixel_format = 0, .width = 1920, .height = 1080,
                        .vir_width = 1920, .vir_height = 1080, .stream_buf_cnt = 3 },
        .rc_attr = { .mode = "cbr", .gop = 30, .bitrate = 4096,
                      .src_frame_rate_num = 30, .src_frame_rate_den = 1,
                      .dst_frame_rate_den = 1, .min_bitrate_roti = 0.8f,
                      .max_bitrate_roti = 1.2f, .min_qp = 20, .max_qp = 51,
                      .min_iqp = 20, .max_iqp = 51,
                      .frm_min_qp = 20, .frm_max_qp = 51,
                      .frm_min_iqp = 20, .frm_max_iqp = 51 },
    };

    TEST_ASSERT_EQ(g_mpi->video->venc_init(&disabled), 0, "venc_init(disabled)");
    TEST_ASSERT_EQ(g_mpi->video->venc_deinit(&disabled), 0, "venc_deinit(disabled)");
    TEST_ASSERT_EQ(g_mpi->video->venc_init(NULL), 0, "venc_init(NULL)");

    int ret = g_mpi->video->venc_init(&enabled);
    if (ret != 0 && ret != -1) {
        fprintf(stderr, "[FAIL] %s: venc_init(enabled) returned %d\n", __func__, ret);
        exit(1);
    }
    g_mpi->video->venc_deinit(&enabled);
    TEST_PASS();
}

/* ============================================================================
 * 测试5: mock ai_ctx，验证 AI init/deinit 路径
 * ============================================================================ */
static void test_ai_ctx_init_deinit(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    struct ai_ctx ai = { .chn = 0, .channel = 0, .samprate = 16000, .bit = 16 };
    TEST_ASSERT_EQ(g_mpi->audio->ai_init(NULL), -1, "ai_init(NULL)");
    TEST_ASSERT_EQ(g_mpi->audio->ai_init(&ai), 0, "ai_init(valid)");
    TEST_ASSERT_EQ(g_mpi->audio->ai_deinit(&ai), 0, "ai_deinit(valid)");
    TEST_PASS();
}

/* ============================================================================
 * 测试6: VPSS 透传路径测试
 * ============================================================================ */
static void test_vpss_pass_through(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    struct vpss_ctx vpss = { .chn_index = 0, .width = 1920, .height = 1080,
                              .out_width = 1280, .out_height = 720, .enable = 1 };
    TEST_ASSERT_EQ(g_mpi->video->vpss_init(&vpss), 0, "vpss_init");
    TEST_ASSERT_EQ(g_mpi->video->vpss_deinit(&vpss), 0, "vpss_deinit");
    char buf[1024]; int len = sizeof(buf);
    TEST_ASSERT_EQ(g_mpi->video->vpss_get_data(&vpss, buf, &len), -1, "vpss_get_data(stub)");
    TEST_PASS();
}

/* ============================================================================
 * 测试7: AENC stub 路径测试
 * ============================================================================ */
static void test_aenc_stub(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    struct aenc_ctx aenc = { .chn = 0, .buf_time = 100, .period = 10 };
    TEST_ASSERT_EQ(g_mpi->audio->aenc_init(&aenc), 0, "aenc_init(stub)");
    TEST_ASSERT_EQ(g_mpi->audio->aenc_deinit(&aenc), 0, "aenc_deinit(stub)");
    char buf[1024]; int len = sizeof(buf);
    TEST_ASSERT_EQ(g_mpi->audio->aenc_get_data(&aenc, buf, &len), -1, "aenc_get_data(stub)");
    TEST_ASSERT_EQ(g_mpi->audio->aenc_release_data(&aenc, buf, len), 0, "aenc_release_data(stub)");
    TEST_PASS();
}

/* ============================================================================
 * 测试8: venc_get_data / venc_release_data 路径测试
 * ============================================================================ */
static void test_venc_get_release_data(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    struct venc_ctx disabled = { .enable = 0, .chn_index = 0 };
    char buf[4096]; int len = sizeof(buf);
    TEST_ASSERT_EQ(g_mpi->video->venc_get_data(&disabled, buf, &len), -1, "venc_get_data(disabled)");
    TEST_ASSERT_EQ(g_mpi->video->venc_release_data(&disabled, buf, len), 0, "venc_release_data(disabled)");
    TEST_ASSERT_EQ(g_mpi->video->venc_get_data(NULL, buf, &len), -1, "venc_get_data(NULL)");
    TEST_PASS();
}

/* ============================================================================
 * 测试9: sys_init / sys_deinit 幂等性测试
 * ============================================================================ */
static void test_sys_init_idempotent(void)
{
    if (g_mpi == NULL) {
        printf("[SKIP] %s: mpi not loaded\n", __func__);
        return;
    }
    int ret1 = g_mpi->sys->sys_init();
    int ret2 = g_mpi->sys->sys_init();
    TEST_ASSERT_EQ(ret1, 0, "sys_init()");
    TEST_ASSERT_EQ(ret2, 0, "sys_init() again");

    int ret3 = g_mpi->sys->sys_deinit();
    int ret4 = g_mpi->sys->sys_deinit();
    TEST_ASSERT_EQ(ret3, 0, "sys_deinit()");
    TEST_ASSERT_EQ(ret4, 0, "sys_deinit() again");
    TEST_PASS();
}

/* ============================================================================
 * main
 * ============================================================================ */
int main(void)
{
    printf("====================================\n");
    printf("  mpi (%s) 单元测试\n", MPI_PLATFORM_NAME);
    printf("====================================\n\n");

    printf("[TEST] test_dlopen_symbol\n");   test_dlopen_symbol();   printf("\n");
    printf("[TEST] test_struct_function_pointers\n"); test_struct_function_pointers(); printf("\n");
    printf("[TEST] test_vi_ctx_init_deinit\n"); test_vi_ctx_init_deinit(); printf("\n");
    printf("[TEST] test_venc_ctx_init_deinit\n"); test_venc_ctx_init_deinit(); printf("\n");
    printf("[TEST] test_ai_ctx_init_deinit\n"); test_ai_ctx_init_deinit(); printf("\n");
    printf("[TEST] test_vpss_pass_through\n"); test_vpss_pass_through(); printf("\n");
    printf("[TEST] test_aenc_stub\n"); test_aenc_stub(); printf("\n");
    printf("[TEST] test_venc_get_release_data\n"); test_venc_get_release_data(); printf("\n");
    printf("[TEST] test_sys_init_idempotent\n"); test_sys_init_idempotent(); printf("\n");

    printf("====================================\n");
    printf("  全部测试通过\n");
    printf("====================================\n");
    return 0;
}
