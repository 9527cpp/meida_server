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
 * @fileName: mpi_stub.c
 * @description: mpi 模拟
 */

#include "mpi_intf.h"
#include <stdio.h>

#define MODULE_TAG "mpi_stub"

#include "log/log_tag.h"

static int stub_sys_init(void)
{
    WriteLog(LOG_INFO, "sys_init");
    return 0;
}

static int stub_sys_deinit(void)
{
    WriteLog(LOG_INFO, "sys_deinit");
    return 0;
}

static int stub_vi_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vi_init");
    return 0;
}

static int stub_vi_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vi_deinit");
    return 0;
}

static int stub_vpss_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vpss_init");
    return 0;
}

static int stub_vpss_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vpss_deinit");
    return 0;
}

static int stub_venc_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "venc_init");
    return 0;
}

static int stub_venc_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "venc_deinit");
    return 0;
}

static int stub_venc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    /* 桩：模拟无数据，返回 -1 让 stream_venc 线程 sleep；实际平台从编码器取数 */
    (void)data;
    (void)len;
    //return -1;
    *len = 100;
    return 0;
}

static int stub_ai_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "ai_init");
    return 0;
}

static int stub_ai_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "ai_deinit");
    return 0;
}

static int stub_aenc_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "aenc_init");
    return 0;
}

static int stub_aenc_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "aenc_deinit");
    return 0;
}

static int stub_aenc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;
}


static struct mpi_sys_opt stub_sys_opt = {
    .sys_init = stub_sys_init,
    .sys_deinit = stub_sys_deinit,
};

static struct mpi_video_opt stub_video_opt = {
    .vi_init = stub_vi_init,
    .vi_deinit = stub_vi_deinit,
    .vpss_init = stub_vpss_init,
    .vpss_deinit = stub_vpss_deinit,
    .venc_init = stub_venc_init,
    .venc_deinit = stub_venc_deinit,
    .venc_get_data = stub_venc_get_data,
};

static struct mpi_audio_opt stub_audio_opt = {
    .ai_init = stub_ai_init,
    .ai_deinit = stub_ai_deinit,
    .aenc_init = stub_aenc_init,
    .aenc_deinit = stub_aenc_deinit,
    .aenc_get_data = stub_aenc_get_data,
};

static struct mpi_intf stub_mpi = {
    .sys = &stub_sys_opt,
    .video = &stub_video_opt,
    .audio = &stub_audio_opt,
};

struct mpi_intf *mpi_intf_create(void)
{
    return &stub_mpi;
}