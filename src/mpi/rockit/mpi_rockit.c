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
 * @description: rockchip mpi 实现
 */

#include "mpi_intf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define MODULE_TAG "mpi_rockit"

#include "log/log_tag.h"

/* Rockchip MPI headers */
#include "rk_type.h"
#include "rk_debug.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_ai.h"
#include "rk_defines.h"

#include <alsa/asoundlib.h>

/* 视频分辨率配置 */
#define VIDEO_WIDTH  1920
#define VIDEO_HEIGHT 1080
#define VIDEO_BITRATE 4096
#define VIDEO_GOP    50
#define VIDEO_FRAMERATE 25

/* 音频配置 */
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_CHANNELS 1
#define AUDIO_PERIOD_SIZE 1024

/* VI 上下文 */
typedef struct {
    int dev_id;
    int pipe_id;
    int chn_id;
    int venc_chn;
    int initialized;
} rockit_vi_ctx;

/* VENC 上下文 */
typedef struct {
    int chn_id;
    int initialized;
    pthread_mutex_t mutex;
} rockit_venc_ctx;

/* AI 上下文 */
typedef struct {
    snd_pcm_t *pcm;
    snd_pcm_hw_params_t *hw_params;
    int initialized;
    pthread_mutex_t mutex;
} rockit_ai_ctx;

/* 全局上下文 */
typedef struct {
    rockit_vi_ctx vi;
    rockit_venc_ctx venc;
    rockit_ai_ctx ai;
    int sys_init;
} rockit_global_ctx;

static rockit_global_ctx g_ctx = {0};

/* 系统初始化 */
static int rockit_sys_init(void)
{
    if (g_ctx.sys_init) {
        WriteLog(LOG_WARNING, "sys already initialized");
        return 0;
    }

    RK_MPI_SYS_Init();
    g_ctx.sys_init = 1;
    WriteLog(LOG_INFO, "sys_init OK");
    return 0;
}

/* 系统反初始化 */
static int rockit_sys_deinit(void)
{
    if (!g_ctx.sys_init) {
        return 0;
    }

    RK_MPI_SYS_Exit();
    g_ctx.sys_init = 0;
    WriteLog(LOG_INFO, "sys_deinit OK");
    return 0;
}

/* VI 初始化 */
static int rockit_vi_init(void *ctx)
{
    (void)ctx;

    if (g_ctx.vi.initialized) {
        WriteLog(LOG_WARNING, "vi already initialized");
        return 0;
    }

    int ret;
    int dev_id = 0;
    int pipe_id = 0;
    int chn_id = 0;

    /* 获取并设置 Dev 属性 */
    VI_DEV_ATTR_S stDevAttr;
    memset(&stDevAttr, 0, sizeof(stDevAttr));
    ret = RK_MPI_VI_GetDevAttr(dev_id, &stDevAttr);
    if (ret == RK_ERR_VI_NOT_CONFIG) {
        ret = RK_MPI_VI_SetDevAttr(dev_id, &stDevAttr);
        if (ret != RK_SUCCESS) {
            WriteLog(LOG_ERROR, "RK_MPI_VI_SetDevAttr failed: %x", ret);
            return -1;
        }
    }

    /* 启用 Dev */
    ret = RK_MPI_VI_GetDevIsEnable(dev_id);
    if (ret != RK_SUCCESS) {
        ret = RK_MPI_VI_EnableDev(dev_id);
        if (ret != RK_SUCCESS) {
            WriteLog(LOG_ERROR, "RK_MPI_VI_EnableDev failed: %x", ret);
            return -1;
        }

        /* 绑定 Dev 和 Pipe */
        VI_DEV_BIND_PIPE_S stBindPipe;
        memset(&stBindPipe, 0, sizeof(stBindPipe));
        stBindPipe.u32Num = 1;
        stBindPipe.PipeId[0] = pipe_id;
        ret = RK_MPI_VI_SetDevBindPipe(dev_id, &stBindPipe);
        if (ret != RK_SUCCESS) {
            WriteLog(LOG_ERROR, "RK_MPI_VI_SetDevBindPipe failed: %x", ret);
            return -1;
        }
    }

    /* 设置通道属性 */
    VI_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));
    stChnAttr.stIspOpt.u32BufCount = 3;
    stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_MMAP;
    stChnAttr.stIspOpt.bNoUseLibV4L2 = (RK_BOOL)1;
    stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
    stChnAttr.stSize.u32Width = VIDEO_WIDTH;
    stChnAttr.stSize.u32Height = VIDEO_HEIGHT;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    stChnAttr.u32Depth = 1;

    ret = RK_MPI_VI_SetChnAttr(dev_id, chn_id, &stChnAttr);
    if (ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VI_SetChnAttr failed: %x", ret);
        return -1;
    }

    ret = RK_MPI_VI_EnableChn(dev_id, chn_id);
    if (ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VI_EnableChn failed: %x", ret);
        return -1;
    }

    g_ctx.vi.dev_id = dev_id;
    g_ctx.vi.pipe_id = pipe_id;
    g_ctx.vi.chn_id = chn_id;
    g_ctx.vi.initialized = 1;

    WriteLog(LOG_INFO, "vi_init OK (dev=%d, pipe=%d, chn=%d)", dev_id, pipe_id, chn_id);
    return 0;
}

/* VI 反初始化 */
static int rockit_vi_deinit(void *ctx)
{
    (void)ctx;

    if (!g_ctx.vi.initialized) {
        return 0;
    }

    RK_MPI_VI_DisableChn(g_ctx.vi.dev_id, g_ctx.vi.chn_id);
    RK_MPI_VI_DisableDev(g_ctx.vi.dev_id);

    memset(&g_ctx.vi, 0, sizeof(g_ctx.vi));
    WriteLog(LOG_INFO, "vi_deinit OK");
    return 0;
}

/* VI 释放数据 */
static void rockit_vi_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
}

/* VPSS 初始化（透传，依赖VI） */
static int rockit_vpss_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vpss_init (pass-through)");
    return 0;
}

/* VPSS 反初始化 */
static int rockit_vpss_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "vpss_deinit");
    return 0;
}

/* VPSS 获取数据（透传） */
static int rockit_vpss_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;
}

/* VPSS 释放数据 */
static void rockit_vpss_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
}

/* VENC 初始化 */
static int rockit_venc_init(void *ctx)
{
    (void)ctx;

    if (g_ctx.venc.initialized) {
        WriteLog(LOG_WARNING, "venc already initialized");
        return 0;
    }

    int ret;
    int venc_chn = 0;

    /* 创建编码通道属性 */
    VENC_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    stChnAttr.stVencAttr.enType = RK_VIDEO_ID_AVC;
    stChnAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
    stChnAttr.stVencAttr.u32PicWidth = VIDEO_WIDTH;
    stChnAttr.stVencAttr.u32PicHeight = VIDEO_HEIGHT;
    stChnAttr.stVencAttr.u32VirWidth = VIDEO_WIDTH;
    stChnAttr.stVencAttr.u32VirHeight = VIDEO_HEIGHT;
    stChnAttr.stVencAttr.u32StreamBufCnt = 5;
    stChnAttr.stVencAttr.u32BufSize = VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2;

    /* H.264 VBR 模式 */
    stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
    stChnAttr.stRcAttr.stH264Vbr.u32BitRate = VIDEO_BITRATE;
    stChnAttr.stRcAttr.stH264Vbr.u32MaxBitRate = VIDEO_BITRATE * 3 / 2;
    stChnAttr.stRcAttr.stH264Vbr.u32MinBitRate = VIDEO_BITRATE / 2;
    stChnAttr.stRcAttr.stH264Vbr.u32Gop = VIDEO_GOP;
    stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateNum = -1;
    stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateDen = 1;
    stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateNum = VIDEO_FRAMERATE;
    stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateDen = 1;

    ret = RK_MPI_VENC_CreateChn(venc_chn, &stChnAttr);
    if (ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VENC_CreateChn failed: %x", ret);
        return -1;
    }

    /* 启动接收帧 */
    VENC_RECV_PIC_PARAM_S stRecvParam;
    memset(&stRecvParam, 0, sizeof(stRecvParam));
    stRecvParam.s32RecvPicNum = -1;
    ret = RK_MPI_VENC_StartRecvFrame(venc_chn, &stRecvParam);
    if (ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VENC_StartRecvFrame failed: %x", ret);
        return -1;
    }

    /* 绑定 VI 到 VENC */
    if (g_ctx.vi.initialized) {
        MPP_CHN_S stSrcChn, stDestChn;
        stSrcChn.enModId = RK_ID_VI;
        stSrcChn.s32DevId = g_ctx.vi.dev_id;
        stSrcChn.s32ChnId = g_ctx.vi.chn_id;

        stDestChn.enModId = RK_ID_VENC;
        stDestChn.s32DevId = venc_chn;
        stDestChn.s32ChnId = venc_chn;

        ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (ret != RK_SUCCESS) {
            WriteLog(LOG_ERROR, "RK_MPI_SYS_Bind VI->VENC failed: %x", ret);
            return -1;
        }
        WriteLog(LOG_INFO, "VI->VENC bind OK");
    }

    pthread_mutex_init(&g_ctx.venc.mutex, NULL);
    g_ctx.venc.chn_id = venc_chn;
    g_ctx.venc.initialized = 1;

    WriteLog(LOG_INFO, "venc_init OK (chn=%d)", venc_chn);
    return 0;
}

/* VENC 反初始化 */
static int rockit_venc_deinit(void *ctx)
{
    (void)ctx;

    if (!g_ctx.venc.initialized) {
        return 0;
    }

    int venc_chn = g_ctx.venc.chn_id;

    RK_MPI_VENC_StopRecvFrame(venc_chn);
    RK_MPI_VENC_DestroyChn(venc_chn);

    pthread_mutex_destroy(&g_ctx.venc.mutex);
    memset(&g_ctx.venc, 0, sizeof(g_ctx.venc));

    WriteLog(LOG_INFO, "venc_deinit OK");
    return 0;
}

/* VENC 获取数据 */
static int rockit_venc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;

    if (!g_ctx.venc.initialized) {
        return -1;
    }

    int venc_chn = g_ctx.venc.chn_id;
    int wait_time = 100;
    int ret;

    VENC_STREAM_S stStream;
    memset(&stStream, 0, sizeof(stStream));
    stStream.pstPack = (VENC_PACK_S *)calloc(1, sizeof(VENC_PACK_S));

    ret = RK_MPI_VENC_GetStream(venc_chn, &stStream, wait_time);
    if (ret == RK_SUCCESS) {
        void *pData = RK_MPI_MB_Handle2VirAddr(stStream.pstPack->pMbBlk);
        if (pData && stStream.pstPack->u32Len > 0) {
            int frame_len = stStream.pstPack->u32Len;
            if (frame_len > *len) {
                frame_len = *len;
            }
            memcpy(data, pData, frame_len);
            *len = frame_len;

            RK_MPI_SYS_MmzFlushCache(stStream.pstPack->pMbBlk, RK_TRUE);
        } else {
            *len = 0;
        }

        RK_MPI_VENC_ReleaseStream(venc_chn, &stStream);
    } else {
        *len = 0;
        ret = -1;
    }

    if (stStream.pstPack) {
        free(stStream.pstPack);
    }

    return ret;
}

/* VENC 释放数据 */
static int rockit_venc_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return 0;
}

/* AI 初始化 */
static int rockit_ai_init(void *ctx)
{
    (void)ctx;

    if (g_ctx.ai.initialized) {
        WriteLog(LOG_WARNING, "ai already initialized");
        return 0;
    }

    int ret;
    snd_pcm_t *pcm;
    snd_pcm_hw_params_t *hw_params;
    unsigned int channels = AUDIO_CHANNELS;
    unsigned int rate = AUDIO_SAMPLE_RATE;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    unsigned int buffer_time = 100000; /* 100ms */
    unsigned int period_time = 20000;  /* 20ms */

    ret = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_open failed: %s", snd_strerror(ret));
        return -1;
    }

    hw_params = (snd_pcm_hw_params_t *)calloc(1, snd_pcm_hw_params_sizeof());
    if (!hw_params) {
        snd_pcm_close(pcm);
        return -1;
    }

    ret = snd_pcm_hw_params_any(pcm, hw_params);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_any failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_set_access failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params_set_format(pcm, hw_params, format);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_set_format failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params_set_channels(pcm, hw_params, channels);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_set_channels failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params_set_rate_near(pcm, hw_params, &rate, 0);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_set_rate_near failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params_set_buffer_time_near(pcm, hw_params, &buffer_time, 0);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_set_buffer_time_near failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params_set_period_time_near(pcm, hw_params, &period_time, 0);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params_set_period_time_near failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_hw_params(pcm, hw_params);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_hw_params failed: %s", snd_strerror(ret));
        goto failed;
    }

    ret = snd_pcm_prepare(pcm);
    if (ret < 0) {
        WriteLog(LOG_ERROR, "snd_pcm_prepare failed: %s", snd_strerror(ret));
        goto failed;
    }

    pthread_mutex_init(&g_ctx.ai.mutex, NULL);
    g_ctx.ai.pcm = pcm;
    g_ctx.ai.hw_params = hw_params;
    g_ctx.ai.initialized = 1;

    WriteLog(LOG_INFO, "ai_init OK");
    return 0;

failed:
    free(hw_params);
    snd_pcm_close(pcm);
    return -1;
}

/* AI 反初始化 */
static int rockit_ai_deinit(void *ctx)
{
    (void)ctx;

    if (!g_ctx.ai.initialized) {
        return 0;
    }

    if (g_ctx.ai.pcm) {
        snd_pcm_drop(g_ctx.ai.pcm);
        snd_pcm_close(g_ctx.ai.pcm);
    }
    if (g_ctx.ai.hw_params) {
        free(g_ctx.ai.hw_params);
    }

    pthread_mutex_destroy(&g_ctx.ai.mutex);
    memset(&g_ctx.ai, 0, sizeof(g_ctx.ai));

    WriteLog(LOG_INFO, "ai_deinit OK");
    return 0;
}

/* AI 获取数据 */
static int rockit_ai_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;

    if (!g_ctx.ai.initialized || !g_ctx.ai.pcm) {
        return -1;
    }

    pthread_mutex_lock(&g_ctx.ai.mutex);

    int frame_size = AUDIO_PERIOD_SIZE * AUDIO_CHANNELS * 2; /* 16bit */
    int ret = snd_pcm_readi(g_ctx.ai.pcm, data, AUDIO_PERIOD_SIZE);

    if (ret == -EAGAIN) {
        pthread_mutex_unlock(&g_ctx.ai.mutex);
        return -1;
    } else if (ret < 0) {
        WriteLog(LOG_WARNING, "snd_pcm_readi error: %s", snd_strerror(ret));
        snd_pcm_prepare(g_ctx.ai.pcm);
        pthread_mutex_unlock(&g_ctx.ai.mutex);
        return -1;
    } else {
        *len = ret * AUDIO_CHANNELS * 2;
    }

    pthread_mutex_unlock(&g_ctx.ai.mutex);
    return 0;
}

/* AI 释放数据 */
static void rockit_ai_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
}

/* AENC 初始化 */
static int rockit_aenc_init(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "aenc_init (not implemented)");
    return 0;
}

/* AENC 反初始化 */
static int rockit_aenc_deinit(void *ctx)
{
    (void)ctx;
    WriteLog(LOG_INFO, "aenc_deinit");
    return 0;
}

/* AENC 获取数据（暂不支持） */
static int rockit_aenc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;
}

/* AENC 释放数据 */
static int rockit_aenc_release_data(void *ctx, void *data, int len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return 0;
}

/* MPI 接口定义 */
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
