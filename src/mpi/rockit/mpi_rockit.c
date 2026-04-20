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
 * @fileName: mpi_rockit.c
 * @description: mpi 模拟
 */

#include "mpi_intf.h"
#include <stdio.h>

#define MODULE_TAG "mpi_rockit"

#include "log/log_tag.h"

static int rockit_sys_init(void)
{
    WriteLog(LOG_INFO, "sys_init");
    return 0;
}

static int rockit_sys_deinit(void)
{
    WriteLog(LOG_INFO, "sys_deinit");
    return 0;
}

static int rockit_vi_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vi_init");
    return 0;
}

static int rockit_vi_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vi_deinit");
    return 0;
}

static void rockit_vi_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
}

static int rockit_vpss_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vpss_init");
    return 0;
}

static int rockit_vpss_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vpss_deinit");
    return 0;
}

static int rockit_vpss_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;
}

static void rockit_vpss_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
}

static int rockit_venc_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "venc_init");
    return 0;
}

static int rockit_venc_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "venc_deinit");
    return 0;
}

static int rockit_venc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    /* 桩：模拟无数据，返回 -1 让 stream_venc 线程 sleep；实际平台从编码器取数 */
    (void)data;
    (void)len;
    //return -1;
    *len = 100;
    return 0;
}

static int rockit_venc_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return 0;
}

static int rockit_ai_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "ai_init");
    return 0;
}

static int rockit_ai_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "ai_deinit");
    return 0;
}

static int rockit_aenc_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "aenc_init");
    return 0;
}

static int rockit_aenc_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "aenc_deinit");
    return 0;
}

static int rockit_aenc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;
}

static void rockit_ai_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
}

static int rockit_aenc_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return 0;
}


static struct mpi_sys_opt rockit_sys_opt = {
    .sys_init = rockit_sys_init,
    .sys_deinit = rockit_sys_deinit,
};

static struct mpi_video_opt rockit_video_opt = {
    .vi_init = rockit_vi_init,
    .vi_deinit = rockit_vi_deinit,
    .vi_release_data = rockit_vi_release_data,
    .vpss_init = rockit_vpss_init,
    .vpss_deinit = rockit_vpss_deinit,
    .vpss_get_data = rockit_vpss_get_data,
    .vpss_release_data = rockit_vpss_release_data,
    .venc_init = rockit_venc_init,
    .venc_deinit = rockit_venc_deinit,
    .venc_get_data = rockit_venc_get_data,
    .venc_release_data = rockit_venc_release_data,
};

static struct mpi_audio_opt rockit_audio_opt = {
    .ai_init = rockit_ai_init,
    .ai_deinit = rockit_ai_deinit,
    .ai_release_data = rockit_ai_release_data,
    .aenc_init = rockit_aenc_init,
    .aenc_deinit = rockit_aenc_deinit,
    .aenc_get_data = rockit_aenc_get_data,
    .aenc_release_data = rockit_aenc_release_data,
};

static struct mpi_intf rockit_mpi = {
    .sys = &rockit_sys_opt,
    .video = &rockit_video_opt,
    .audio = &rockit_audio_opt,
};

struct mpi_intf *mpi_intf_create(void)
{
    return &rockit_mpi;
}