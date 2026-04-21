/**
 * Copyright (c) 2026 Oray Inc. All rights reserved.
 *
 * @author: LiJun
 * @date: 2026-04-21
 * @fileName: demo_mpi.c
 * @description: MPI 各平台全流程演示
 *
 * 演示 mpi_intf 从创建到取帧到释放的完整生命周期：
 *   1. dlopen .so，dlsym mpi_intf_create
 *   2. 创建 mpi_intf 实例
 *   3. 构造各通道上下文（VI/VENC/AI）
 *   4. 调用 init / get_data / release_data / deinit
 *   5. 释放资源
 *
 * 宏控选择测试平台：
 *   -DMPI_PLATFORM_ROCKIT  -> demo mpi_rockit.so
 *   -DMPI_PLATFORM_HISI    -> demo mpi_hisi.so
 *   -DMPI_PLATFORM_RKMEDIA -> demo mpi_rkmedia.so
 *   -DMPI_PLATFORM_STUB    -> demo mpi_stub.so
 *
 * 编译：
 *   cd mpi/build && gcc -o demo_mpi_stub ../demo_mpi.c -DMPI_PLATFORM_STUB \
 *       -I.. -I../../mpi_ctx -I../../log -ldl -lm -lpthread
 *
 * 运行（需在 mpi/build/ 目录下）：
 *   ./demo_mpi_stub
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>

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
 * 辅助函数
 * ============================================================================ */
static void *load_mpi_so(void)
{
    const char *paths[] = { "./" MPI_SO_NAME, "../build/" MPI_SO_NAME, NULL };
    for (int i = 0; paths[i]; i++) {
        void *h = dlopen(paths[i], RTLD_NOW);
        if (h) {
            printf("[DEMO] dlopen %s OK\n", paths[i]);
            return h;
        }
    }
    fprintf(stderr, "[DEMO] dlopen %s failed: %s\n", MPI_SO_NAME, dlerror());
    return NULL;
}

static struct mpi_intf *create_mpi(void *handle)
{
    (void)dlerror();
    mpi_intf_create_fn create_fn = dlsym(handle, MPI_CREATE_SYM);
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "[DEMO] dlsym(%s) failed: %s\n", MPI_CREATE_SYM, err);
        return NULL;
    }
    struct mpi_intf *mpi = create_fn();
    if (!mpi) {
        fprintf(stderr, "[DEMO] mpi_intf_create() returned NULL\n");
        return NULL;
    }
    printf("[DEMO] mpi_intf_create() OK\n");
    return mpi;
}

/* ============================================================================
 * Demo 1: VI 通道全流程
 * ============================================================================ */
static void demo_vi_channel(struct mpi_intf *mpi)
{
    printf("\n========== Demo VI Channel ==========\n");

    struct vi_ctx vi = {
        .enable = 1,
        .devid = 0,
        .pipeid = 0,
        .chn_index = 0,
        .entity_name = "virt",
        .format = 0,
        .width = 1920,
        .height = 1080,
        .buf_cnt = 3,
        .memory_type = 0,
    };

    printf("[DEMO] vi_init ...\n");
    int ret = mpi->video->vi_init(&vi);
    printf("[DEMO] vi_init -> %d\n", ret);

    printf("[DEMO] vi_deinit ...\n");
    ret = mpi->video->vi_deinit(&vi);
    printf("[DEMO] vi_deinit -> %d\n", ret);

    printf("[DEMO] VI channel demo done\n");
}

/* ============================================================================
 * Demo 2: VENC 通道全流程（init -> get_data -> release_data -> deinit）
 * ============================================================================ */
static void demo_venc_channel(struct mpi_intf *mpi)
{
    printf("\n========== Demo VENC Channel ==========\n");

    struct venc_ctx venc = {
        .enable = 1,
        .chn_index = 0,
        .type = "h265",
        .venc_attr = {
            .pixel_format = 0,
            .width = 1920,
            .height = 1080,
            .vir_width = 1920,
            .vir_height = 1080,
            .stream_buf_cnt = 3,
        },
        .rc_attr = {
            .mode = "cbr",
            .gop = 30,
            .bitrate = 4096,
            .src_frame_rate_num = 30,
            .src_frame_rate_den = 1,
            .dst_frame_rate_den = 1,
            .min_bitrate_roti = 0.8f,
            .max_bitrate_roti = 1.2f,
            .min_qp = 20,
            .max_qp = 51,
            .min_iqp = 20,
            .max_iqp = 51,
            .frm_min_qp = 20,
            .frm_max_qp = 51,
            .frm_min_iqp = 20,
            .frm_max_iqp = 51,
        },
    };

    printf("[DEMO] venc_init ...\n");
    int ret = mpi->video->venc_init(&venc);
    printf("[DEMO] venc_init -> %d\n", ret);

    /* 尝试取 3 帧演示 */
    for (int i = 0; i < 3; i++) {
        char buf[8192];
        int len = sizeof(buf);
        ret = mpi->video->venc_get_data(&venc, buf, &len);
        printf("[DEMO] venc_get_data frame[%d] -> ret=%d len=%d\n", i, ret, len);
        if (ret == 0 && len > 0) {
            int rel = mpi->video->venc_release_data(&venc, buf, len);
            printf("[DEMO] venc_release_data -> %d\n", rel);
        }
        if (ret == 0) {
            usleep(40000); /* ~25fps, simulate real timing */
        }
    }

    printf("[DEMO] venc_deinit ...\n");
    ret = mpi->video->venc_deinit(&venc);
    printf("[DEMO] venc_deinit -> %d\n", ret);

    printf("[DEMO] VENC channel demo done\n");
}

/* ============================================================================
 * Demo 3: AI 通道全流程
 * ============================================================================ */
static void demo_ai_channel(struct mpi_intf *mpi)
{
    printf("\n========== Demo AI Channel ==========\n");

    struct ai_ctx ai = {
        .chn = 0,
        .channel = 0,
        .samprate = 16000,
        .bit = 16,
    };

    printf("[DEMO] ai_init ...\n");
    int ret = mpi->audio->ai_init(&ai);
    printf("[DEMO] ai_init -> %d\n", ret);

    /* 尝试取 3 次音频数据演示 */
    for (int i = 0; i < 3; i++) {
        char buf[4096];
        int len = sizeof(buf);
        if (mpi->audio->ai_get_data) {
            ret = mpi->audio->ai_get_data(&ai, buf, &len);
            printf("[DEMO] ai_get_data sample[%d] -> ret=%d len=%d\n", i, ret, len);
            if (ret == 0 && len > 0) {
                mpi->audio->ai_release_data(&ai, buf, len);
            }
        } else {
            printf("[DEMO] ai_get_data not supported (NULL)\n");
            break;
        }
    }

    printf("[DEMO] ai_deinit ...\n");
    ret = mpi->audio->ai_deinit(&ai);
    printf("[DEMO] ai_deinit -> %d\n", ret);

    printf("[DEMO] AI channel demo done\n");
}

/* ============================================================================
 * Demo 4: AENC 通道全流程
 * ============================================================================ */
static void demo_aenc_channel(struct mpi_intf *mpi)
{
    printf("\n========== Demo AENC Channel ==========\n");

    struct aenc_ctx aenc = {
        .chn = 0,
        .buf_time = 100,
        .period = 10,
    };

    printf("[DEMO] aenc_init ...\n");
    int ret = mpi->audio->aenc_init(&aenc);
    printf("[DEMO] aenc_init -> %d\n", ret);

    /* 尝试取编码数据演示 */
    for (int i = 0; i < 3; i++) {
        char buf[4096];
        int len = sizeof(buf);
        ret = mpi->audio->aenc_get_data(&aenc, buf, &len);
        printf("[DEMO] aenc_get_data packet[%d] -> ret=%d len=%d\n", i, ret, len);
        if (ret == 0 && len > 0) {
            mpi->audio->aenc_release_data(&aenc, buf, len);
        }
    }

    printf("[DEMO] aenc_deinit ...\n");
    ret = mpi->audio->aenc_deinit(&aenc);
    printf("[DEMO] aenc_deinit -> %d\n", ret);

    printf("[DEMO] AENC channel demo done\n");
}

/* ============================================================================
 * Demo 5: VPSS 通道全流程
 * ============================================================================ */
static void demo_vpss_channel(struct mpi_intf *mpi)
{
    printf("\n========== Demo VPSS Channel ==========\n");

    struct vpss_ctx vpss = {
        .chn_index = 0,
        .width = 1920,
        .height = 1080,
        .out_width = 1280,
        .out_height = 720,
        .enable = 1,
    };

    printf("[DEMO] vpss_init ...\n");
    int ret = mpi->video->vpss_init(&vpss);
    printf("[DEMO] vpss_init -> %d\n", ret);

    char buf[8192];
    int len = sizeof(buf);
    ret = mpi->video->vpss_get_data(&vpss, buf, &len);
    printf("[DEMO] vpss_get_data -> ret=%d len=%d\n", ret, len);

    mpi->video->vpss_release_data(&vpss, buf, len);
    printf("[DEMO] vpss_release_data done\n");

    printf("[DEMO] vpss_deinit ...\n");
    ret = mpi->video->vpss_deinit(&vpss);
    printf("[DEMO] vpss_deinit -> %d\n", ret);

    printf("[DEMO] VPSS channel demo done\n");
}

/* ============================================================================
 * Demo 6: sys init / deinit
 * ============================================================================ */
static void demo_sys_init_deinit(struct mpi_intf *mpi)
{
    printf("\n========== Demo SYS Init/Deinit ==========\n");

    printf("[DEMO] sys_init ...\n");
    int ret = mpi->sys->sys_init();
    printf("[DEMO] sys_init -> %d\n", ret);

    printf("[DEMO] sys_init again (idempotent) ...\n");
    ret = mpi->sys->sys_init();
    printf("[DEMO] sys_init again -> %d\n", ret);

    printf("[DEMO] sys_deinit ...\n");
    ret = mpi->sys->sys_deinit();
    printf("[DEMO] sys_deinit -> %d\n", ret);

    printf("[DEMO] sys_deinit again ...\n");
    ret = mpi->sys->sys_deinit();
    printf("[DEMO] sys_deinit again -> %d\n", ret);

    printf("[DEMO] SYS demo done\n");
}

/* ============================================================================
 * main
 * ============================================================================ */
int main(void)
{
    printf("========================================\n");
    printf("  mpi (%s) 全流程演示\n", MPI_PLATFORM_NAME);
    printf("========================================\n");

    /* 1. 加载 .so */
    void *handle = load_mpi_so();
    if (!handle) {
        fprintf(stderr, "[DEMO] FAILED: cannot load %s\n", MPI_SO_NAME);
        return 1;
    }

    /* 2. 创建 mpi_intf 实例 */
    struct mpi_intf *mpi = create_mpi(handle);
    if (!mpi) {
        dlclose(handle);
        return 1;
    }

    /* 3. 各通道演示 */
    demo_sys_init_deinit(mpi);
    demo_vi_channel(mpi);
    demo_venc_channel(mpi);
    demo_vpss_channel(mpi);
    demo_ai_channel(mpi);
    demo_aenc_channel(mpi);

    printf("\n========================================\n");
    printf("  Demo 全部完成\n");
    printf("========================================\n");

    /* 4. 释放（不 dlclose，保持 mpi 指针有效） */
    (void)handle;
    return 0;
}
