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
#include "mpi_ctx/mpi_ctx_intf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MODULE_TAG "mpi_rockit"

#include "log/log_tag.h"

/* Rockchip MPI headers */
#include "rk_type.h"
#include "rk_debug.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_mb.h"
#include "rk_defines.h"

// #include <alsa/asoundlib.h>  /* TODO: re-enable when alsa available */

/* VENC 通道私有数据 */
typedef struct {
    int venc_chn;
    int initialized;
    pthread_mutex_t mutex;
} rockit_venc_priv;

/* AI 通道私有数据 - TODO: re-enable when alsa available */
/* typedef struct {
    snd_pcm_t *pcm;
    snd_pcm_hw_params_t *hw_params;
    int initialized;
    pthread_mutex_t mutex;
} rockit_ai_priv; */

static int g_sys_init = 0;

/* 系统初始化 */
static int rockit_sys_init(void)
{
    if (g_sys_init) {
        WriteLog(LOG_WARNING, "sys already initialized");
        return 0;
    }

    RK_MPI_SYS_Init();
    g_sys_init = 1;
    WriteLog(LOG_INFO, "sys_init OK");
    return 0;
}

/* 系统反初始化 */
static int rockit_sys_deinit(void)
{
    if (!g_sys_init) {
        return 0;
    }

    RK_MPI_SYS_Exit();
    g_sys_init = 0;
    WriteLog(LOG_INFO, "sys_deinit OK");
    return 0;
}

/* VI 初始化 */
static int rockit_vi_init(void *ctx)
{
    struct vi_ctx *vi = (struct vi_ctx *)ctx;
    if (!vi || !vi->enable) {
        return 0;
    }

    int ret;
    int dev_id = vi->devid;
    int pipe_id = vi->pipeid;
    int chn_id = vi->chn_index;

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
    stChnAttr.stIspOpt.u32BufCount = vi->buf_cnt > 0 ? vi->buf_cnt : 3;
    stChnAttr.stIspOpt.enMemoryType = (enum rkVI_V4L2_MEMORY_TYPE)vi->memory_type;
    stChnAttr.stIspOpt.bNoUseLibV4L2 = (RK_BOOL)1;
    stChnAttr.enPixelFormat = (PIXEL_FORMAT_E)vi->format;
    stChnAttr.stSize.u32Width = vi->width;
    stChnAttr.stSize.u32Height = vi->height;
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

    WriteLog(LOG_INFO, "vi_init OK (dev=%d, pipe=%d, chn=%d, %dx%d)",
             dev_id, pipe_id, chn_id, vi->width, vi->height);
    return 0;
}

/* VI 反初始化 */
static int rockit_vi_deinit(void *ctx)
{
    struct vi_ctx *vi = (struct vi_ctx *)ctx;
    if (!vi || !vi->enable) {
        return 0;
    }

    RK_MPI_VI_DisableChn(vi->devid, vi->chn_index);
    RK_MPI_VI_DisableDev(vi->devid);

    WriteLog(LOG_INFO, "vi_deinit OK (dev=%d, chn=%d)", vi->devid, vi->chn_index);
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
    struct venc_ctx *venc = (struct venc_ctx *)ctx;
    if (!venc || !venc->enable) {
        return 0;
    }

    int ret;
    int venc_chn = venc->chn_index;

    /* 创建编码通道属性 */
    VENC_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    /* 编码类型 */
    if (strcmp(venc->type, "h265") == 0) {
        stChnAttr.stVencAttr.enType = RK_VIDEO_ID_HEVC;
    } else if (strcmp(venc->type, "mjpeg") == 0) {
        stChnAttr.stVencAttr.enType = RK_VIDEO_ID_MJPEG;
    } else {
        stChnAttr.stVencAttr.enType = RK_VIDEO_ID_AVC; /* default h264 */
    }

    stChnAttr.stVencAttr.enPixelFormat = (PIXEL_FORMAT_E)venc->venc_attr.pixel_format;
    stChnAttr.stVencAttr.u32PicWidth = venc->venc_attr.width;
    stChnAttr.stVencAttr.u32PicHeight = venc->venc_attr.height;
    stChnAttr.stVencAttr.u32VirWidth = venc->venc_attr.vir_width;
    stChnAttr.stVencAttr.u32VirHeight = venc->venc_attr.vir_height;
    stChnAttr.stVencAttr.u32StreamBufCnt = venc->venc_attr.stream_buf_cnt;
    stChnAttr.stVencAttr.u32BufSize = venc->venc_attr.width * venc->venc_attr.height * 3 / 2;

    /* RC 模式 */
    if (strcmp(venc->rc_attr.mode, "cbr") == 0) {
        if (stChnAttr.stVencAttr.enType == RK_VIDEO_ID_AVC) {
            stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
            stChnAttr.stRcAttr.stH264Cbr.u32BitRate = venc->rc_attr.bitrate;
            stChnAttr.stRcAttr.stH264Cbr.u32Gop = venc->rc_attr.gop;
            stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = venc->rc_attr.src_frame_rate_num;
            stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = venc->rc_attr.src_frame_rate_den;
            stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = venc->rc_attr.dst_frame_rate_den > 0 ? (venc->rc_attr.gop / venc->rc_attr.dst_frame_rate_den) : 25;
            stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
        } else if (stChnAttr.stVencAttr.enType == RK_VIDEO_ID_HEVC) {
            stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
            stChnAttr.stRcAttr.stH265Cbr.u32BitRate = venc->rc_attr.bitrate;
            stChnAttr.stRcAttr.stH265Cbr.u32Gop = venc->rc_attr.gop;
        }
    } else {
        /* vbr */
        if (stChnAttr.stVencAttr.enType == RK_VIDEO_ID_AVC) {
            stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
            stChnAttr.stRcAttr.stH264Vbr.u32BitRate = venc->rc_attr.bitrate;
            stChnAttr.stRcAttr.stH264Vbr.u32MaxBitRate = (int)(venc->rc_attr.bitrate * venc->rc_attr.max_bitrate_roti);
            stChnAttr.stRcAttr.stH264Vbr.u32MinBitRate = (int)(venc->rc_attr.bitrate * venc->rc_attr.min_bitrate_roti);
            stChnAttr.stRcAttr.stH264Vbr.u32Gop = venc->rc_attr.gop;
            stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateNum = venc->rc_attr.src_frame_rate_num;
            stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateDen = venc->rc_attr.src_frame_rate_den;
            stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateNum = venc->rc_attr.dst_frame_rate_den > 0 ? (venc->rc_attr.gop / venc->rc_attr.dst_frame_rate_den) : 25;
            stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateDen = 1;
        } else if (stChnAttr.stVencAttr.enType == RK_VIDEO_ID_HEVC) {
            stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stH265Vbr.u32BitRate = venc->rc_attr.bitrate;
            stChnAttr.stRcAttr.stH265Vbr.u32MaxBitRate = (int)(venc->rc_attr.bitrate * venc->rc_attr.max_bitrate_roti);
            stChnAttr.stRcAttr.stH265Vbr.u32MinBitRate = (int)(venc->rc_attr.bitrate * venc->rc_attr.min_bitrate_roti);
        }
    }

    ret = RK_MPI_VENC_CreateChn(venc_chn, &stChnAttr);
    if (ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VENC_CreateChn failed: %x", ret);
        return -1;
    }

    /* 设置 QP 参数 */
    VENC_RC_PARAM_S stRcParam;
    memset(&stRcParam, 0, sizeof(stRcParam));
    stRcParam.stParamH264.u32MaxQp = venc->rc_attr.max_qp;
    stRcParam.stParamH264.u32MinQp = venc->rc_attr.min_qp;
    stRcParam.stParamH264.u32MaxIQp = venc->rc_attr.max_iqp;
    stRcParam.stParamH264.u32MinIQp = venc->rc_attr.min_iqp;
    stRcParam.stParamH264.u32FrmMaxQp = venc->rc_attr.frm_max_qp;
    stRcParam.stParamH264.u32FrmMinQp = venc->rc_attr.frm_min_qp;
    stRcParam.stParamH264.u32FrmMaxIQp = venc->rc_attr.frm_max_iqp;
    stRcParam.stParamH264.u32FrmMinIQp = venc->rc_attr.frm_min_iqp;
    RK_MPI_VENC_SetRcParam(venc_chn, &stRcParam);

    /* 启动接收帧 */
    VENC_RECV_PIC_PARAM_S stRecvParam;
    memset(&stRecvParam, 0, sizeof(stRecvParam));
    stRecvParam.s32RecvPicNum = -1;
    ret = RK_MPI_VENC_StartRecvFrame(venc_chn, &stRecvParam);
    if (ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VENC_StartRecvFrame failed: %x", ret);
        return -1;
    }

    WriteLog(LOG_INFO, "venc_init OK (chn=%d, %dx%d, %s, %s %dkbps)",
             venc_chn,
             venc->venc_attr.width, venc->venc_attr.height,
             venc->type, venc->rc_attr.mode, venc->rc_attr.bitrate);
    return 0;
}

/* VENC 反初始化 */
static int rockit_venc_deinit(void *ctx)
{
    struct venc_ctx *venc = (struct venc_ctx *)ctx;
    if (!venc || !venc->enable) {
        return 0;
    }

    int venc_chn = venc->chn_index;

    RK_MPI_VENC_StopRecvFrame(venc_chn);
    RK_MPI_VENC_DestroyChn(venc_chn);

    WriteLog(LOG_INFO, "venc_deinit OK (chn=%d)", venc_chn);
    return 0;
}

/* VENC 获取数据 */
static int rockit_venc_get_data(void *ctx, void *data, int *len)
{
    struct venc_ctx *venc = (struct venc_ctx *)ctx;
    if (!venc || !venc->enable) {
        return -1;
    }

    int venc_chn = venc->chn_index;
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

/* AI 初始化 - TODO: re-enable alsa when available */
static int rockit_ai_init(void *ctx)
{
    struct ai_ctx *ai = (struct ai_ctx *)ctx;
    if (!ai) {
        return -1;
    }
    WriteLog(LOG_INFO, "ai_init (stub, ch=%d)", ai->chn);
    return 0;
}

/* AI 反初始化 */
static int rockit_ai_deinit(void *ctx)
{
    struct ai_ctx *ai = (struct ai_ctx *)ctx;
    if (!ai) {
        return 0;
    }
    WriteLog(LOG_INFO, "ai_deinit OK (ch=%d)", ai->chn);
    return 0;
}

/* AI 获取数据 */
static int rockit_ai_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;  /* stub: no data */
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
