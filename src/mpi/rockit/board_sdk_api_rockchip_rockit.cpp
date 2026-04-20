/*
 * Copyright (c) 2021-2025 Oray Inc. All rights reserved.
 *
 * No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Oray Inc.
 *
 * @author:
 * @description: soc related
 */

#include "board-sdk/board_sdk_api.hpp"
#include "board-sdk/board_sdk_tool.hpp"
#include "logger/phlogger.h"
#include "common/kvmConfig.h"

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <limits.h>

#include "rk_debug.h"
#include "rk_mpi_vi.h"
#include "vi.h"

#include "rk_debug.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"

#include "rk_defines.h"
#include "rk_debug.h"
#include "rk_mpi_ai.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"



#include "libalsaWrapper/alsa-wrapper.h"

#ifdef __cplusplus
extern "C"{
#endif
#include "libavformat/avformat.h"
#include "libavformat/version.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#ifdef __cplusplus
}
#endif

#ifndef VDEC_INT64_MIN
#define VDEC_INT64_MIN               (-0x7fffffffffffffffLL-1)
#endif

#ifndef VDEC_INT64_MAX
#define VDEC_INT64_MAX               INT64_C(9223372036854775807)
#endif

#define ENABLE_RTSP 0
#define ENABLE_AUTO_FILE 1

#if ENABLE_RTSP
HANDLE handle;
#define RTSP_PORT 554
#endif

bool is_vi_chn_enable = false;
bool is_venc_chn_enable = false;
Calsa_wrapper *_alsa_capture = NULL;
Calsa_wrapper *_alsa_MIC_capture = NULL;
alsa_hwparams *_alsa_param = NULL;
alsa_hwparams *_alsa_MIC_param = NULL;

using namespace std;


typedef enum {
	EM_VI_VPSS_VO=0,
	EM_VI_VPSS_TDE,
	EM_VI_VPSS_VENC,
	EM_VI_GET_VENC_SEND,
	EM_RTSP_VDEC_VPSS_TDE_VENC_VO, 
	EM_RTSP_VDEC_VPSS_VENC_VO,
	EM_FILE_VDEC_VPSS_VENC,
	EM_VI_VENC_RTSP,
	EM_VI_VENC_AVS_RTSP,
	EM_BUTT
}TEST_TYPE;
	
struct argsv{
	TEST_TYPE arg;
	const char *usage;
};




/* void video_packet_cb(MEDIA_BUFFER mb, RK_VOID *pHandle); */ 

static unsigned char p_sei_[55] = {
    0x00, 0x00, 0x00, 0x01, 0x06, 0x05, 0x2F, 0xDC,   0x45, 0xE9, 0xBD, 0xE6, 0xD9, 0x48, 0xB7, 0x96,
    0x2C, 0xD8, 0x20, 0xD9, 0x23, 0xEE, 0xEF, 0x5A,   0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x72, 0x20,
    0x56, 0x69, 0x64, 0x65, 0xFD, 0x20, 0x45, 0x6E,   0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x20, 0x53,
    0x79, 0x73, 0x74, 0x65, 0x6D, 0x00, 0x80
};

void board_sdk_api::frame_send_status_cb(int err)
{
    return;
    static struct timeval last_time, now_time;

    gettimeofday(&now_time,NULL);
    int sleep_time = now_time.tv_sec*1000 + now_time.tv_usec/1000 - (last_time.tv_sec*1000 + last_time.tv_usec/1000);
    if ( sleep_time < 1000 * 10)
    {
        return;
    }

    if(err)
    {
        gettimeofday(&last_time,NULL);
        last_image_quality_level = image_quality_level;
        image_quality_level--;
        is_image_quality_up = true;

        int bit_rate = config.video[0].video_out.bitrate - 128;
        
        set_image_quality(config.video[0].video_out.frame_rate, bit_rate);

    }
    else
    {
        if ( last_image_quality_level - 1 == image_quality_level && is_image_quality_up)
        {
            gettimeofday(&last_time,NULL);
            int frame_rate_span = 0;
            int bit_rate_span = 0;
            is_image_quality_up = false;

            int bit_rate = config.video[0].video_out.bitrate + 32;
            set_image_quality(config.video[0].video_out.frame_rate, bit_rate);
        }
    }

    return;
}



bool board_sdk_api::rset_image_quality_level(int level)
{
    image_quality_level = level;
    p_frame_rate_and_bit_rate p_image_quality =  get_recommend_image_quality(image_quality_level);

    return set_image_quality(p_image_quality->frame_rate, p_image_quality->bit_rate);
}

bool board_sdk_api::_restart_recv_frame() {
	VENC_RECV_PIC_PARAM_S stRecvParam;
	RK_U32 ret = -1;

	ret = RK_MPI_VENC_StopRecvFrame(0);
	if (ret != RK_SUCCESS) {
		WriteLog(LOG_ERROR, "RK_MPI_VENC_StopRecvFrame failed: %lu", ret);
		return false;
	}

	memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));
	stRecvParam.s32RecvPicNum = -1;

	ret = RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);
	if (ret != RK_SUCCESS) {
		WriteLog(LOG_ERROR, "RK_MPI_VENC_StartRecvFrame failed: %lu", ret);
		return false;
	}

	return true;
}

bool board_sdk_api::set_frame_info(const frame_info fi) {
	int frame_rate = fi.framerate;
	int bitrate_level = fi.bitrate_level;
	int bitrate = 0;

	// 如果是设置了bitrate_level
	if (bitrate_level >= LEVEL_MIN && bitrate_level <= LEVEL_MAX) {
		int bitrate_step = (bitrate_max - bitrate_min) / (LEVEL_MAX - LEVEL_MIN);
		bitrate = bitrate_min + bitrate_step * (bitrate_level - LEVEL_MIN);
	} else {
		/*
		 * 如果没设置 bitrate_level,则使用bitrate,为了兼容 网页上使用的设置是bitrate, 向日葵客户端设置的是bitrate_level
		 */
		bitrate = fi.bitrate;
	}
	WriteLog(LOG_DEBUG, "bitrate_level: %d, bitrate: %d", bitrate_level, bitrate);

	// 要判断是否设置了frame_rate
	if (frame_rate != 0) {

		// 帧率实时生效
		config.video[0].video_out.frame_rate = (frame_rate > framerate_max) ? framerate_max : frame_rate;
		save_param("oraykvm.screen.h264_enc_dest_frame", config.video[0].video_out.frame_rate);

		WriteLog(LOG_DEBUG, "framerate: %d, framerate_max: %d, framerate_used: %d", frame_rate, framerate_max,
				config.video[0].video_out.frame_rate
				 );
	}

	// 要判断是否设置了bitrate
	if (bitrate != 0) {

		config.video[0].video_out.bitrate = bitrate;
		save_param("oraykvm.screen.h264_enc_rate", config.video[0].video_out.bitrate);

		RK_U32 ret = -1;
		VENC_CHN_ATTR_S stChnAttr;

		memset(&stChnAttr, 0, sizeof(VENC_CHN_ATTR_S));

		/* 
		 * 先获取当前编码器属性,用于判断当前编码器类型
		 */
		ret = RK_MPI_VENC_GetChnAttr(0, &stChnAttr);
		if (ret != RK_SUCCESS) {
			WriteLog(LOG_ERROR, "RK_MPI_VENC_GetChnAttr failed: %lu", ret);
			return false;
		}

		if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264CBR
			|| stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR) {
			/*
			 * 简便起见,一起设置,反正只会有一个生效
			 */
			stChnAttr.stRcAttr.stH264Cbr.u32BitRate = bitrate;
			stChnAttr.stRcAttr.stH264Vbr.u32BitRate = bitrate;
			// 注意这个也得设置
			stChnAttr.stRcAttr.stH264Vbr.u32MaxBitRate = bitrate * 3 / 2;
			stChnAttr.stRcAttr.stH264Vbr.u32MinBitRate = bitrate / 2;
		}

		if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR
			|| stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR) {
			/*
			 * 简便起见,一起设置,反正只会有一个生效
			 */
			stChnAttr.stRcAttr.stH265Cbr.u32BitRate = bitrate;
			stChnAttr.stRcAttr.stH265Vbr.u32BitRate = bitrate;
			// 注意这个也得设置
			stChnAttr.stRcAttr.stH265Vbr.u32MaxBitRate = bitrate * 3 / 2;
			stChnAttr.stRcAttr.stH265Vbr.u32MinBitRate = bitrate / 2;
		}

		ret = RK_MPI_VENC_SetChnAttr(0, &stChnAttr);
		if (ret != RK_SUCCESS) {
			WriteLog(LOG_ERROR, "RK_MPI_VENC_SetChnAttr failed: %lu", ret);
			return false;
		}

		return _restart_recv_frame();
	}
	return true;
}

void board_sdk_api::get_frame_info(frame_info &fi) {
	int frame_rate = config.video[0].video_out.frame_rate;
	int bitrate = config.video[0].video_out.bitrate;
	int bl;

	fi.framerate = frame_rate;

	if (bitrate <= bitrate_min) {
		bl = LEVEL_MIN;
	} else if (bitrate >= bitrate_max) {
		bl = LEVEL_MAX;
	} else {
		int bitrate_step = (bitrate_max - bitrate_min) / (LEVEL_MAX - LEVEL_MIN);
		bl = ((bitrate - bitrate_min) / bitrate_step) + 1;
	}

	fi.bitrate_level = bl;

	/*
	 * TODO: 后续如有扩展这里添加其它信息
	 */
}


/*
 * focus: this function is deprecated
 */
bool board_sdk_api::set_image_quality(int frame_rate, int bit_rate)
{
    if ( frame_rate > 1)
    {
        if (frame_rate > 25)
            frame_rate = 25;

        config.video[0].video_out.frame_rate = frame_rate;     ///<  视频通道输出流参数 帧率
    }
        
    if ( bit_rate > 0  && bit_rate < 1024 * 10)
	    config.video[0].video_out.bitrate = bit_rate;	        ///<  视频通道输出流参数 码率(单位Kbits)

    if ( !stream_on_ )
    {
        return false;
    }

    return true;
    
}

bool board_sdk_api::_board_hardware_init()
{
    RK_MPI_SYS_Init();
    return true;
}

void board_sdk_api::_board_hardware_uninit()
{
    RK_MPI_SYS_Exit();
}

#define TEST_VENC_MAX 2
#define TEST_WITH_FD 0
#define TEST_WITH_FD_SWITCH 0
#define DEBUG_USB_CAMERA_HPCLASS_20230614_ 0

/* Defines the features of an frame */
typedef struct rkVI_FRAME_S {
    MB_BLK            pMbBlk;                     /* R; the mediabuf of frame */
    RK_U32            u32UniqueId;                /* R; the UniqueId of frame */
    RK_U32            u32Fd;                      /* R; the fd of frame */
    RK_U32            u32Len;                     /* R; the length of frame */
    RK_S64            s64PTS;                     /* R; PTS */
    RK_S32            s32Seq;                     /* R; the list number of frame*/
    PIXEL_FORMAT_E    enPixelFormat;              /* R; the pixel format */
    SIZE_S            stSize;                     /* R; the frame out put size */
    COMPRESS_MODE_E   enCompressMode;             /* R; 256B Segment compress or no compress. */
} VI_FRAME_S;
typedef struct _rkTestVencCfg {
    RK_BOOL bOutDebugCfg;
    VENC_CHN_ATTR_S stAttr;
    RK_CHAR dstFilePath[128];
    RK_CHAR dstFileName[128];
    RK_S32 s32ChnId;
    FILE *fp;
    RK_S32 selectFd;
    VENC_RC_PARAM_S stRcParam;
} TEST_VENC_CFG;

typedef enum rkTestVIMODE_E {
    TEST_VI_MODE_VI_ONLY = 0,
    TEST_VI_MODE_BIND_VENC = 1,
    TEST_VI_MODE_BIND_VENC_MULTI = 2,
} TEST_VI_MODE_E;
typedef struct _rkMpiVICtx {
    RK_S32 width;
    RK_S32 height;
    RK_S32 devId;
    RK_S32 pipeId;
    RK_S32 channelId;
    RK_S32 loopCountSet;
    RK_S32 selectFd;
    VI_DEV_ATTR_S stDevAttr;
    VI_DEV_BIND_PIPE_S stBindPipe;
    VI_CHN_ATTR_S stChnAttr;
    VI_SAVE_FILE_INFO_S stDebugFile;
    VI_FRAME_S stViFrame;
    VI_CHN_STATUS_S stChnStatus;
    TEST_VI_MODE_E enMode;
    const char *aEntityName;
    // for venc
    TEST_VENC_CFG stVencCfg[TEST_VENC_MAX];
    VENC_STREAM_S stFrame[TEST_VENC_MAX];
	MPP_CHN_S stDestChn[TEST_VENC_MAX];
	MPP_CHN_S stSrcChn[TEST_VENC_MAX];
	
    RK_U32 s32FirstFrameStartQp;
    RK_U32 u32StepQp;
    RK_U32 u32MaxQp;
    RK_U32 u32MinQp;
#if DEBUG_USB_CAMERA_HPCLASS_20230614_
	FILE *h264_fp;
#endif
} TEST_VI_CTX_S;

static void mpi_vi_test_show_options(const TEST_VI_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");

    RK_PRINT("output file open      : %d\n", ctx->stDebugFile.bCfg);
    RK_PRINT("yuv output file name  : %s/%s\n", ctx->stDebugFile.aFilePath, ctx->stDebugFile.aFileName);
    RK_PRINT("enc0 output file path  : /%s/%s\n", ctx->stVencCfg[0].dstFilePath, ctx->stVencCfg[0].dstFileName);
    RK_PRINT("enc1 output file path  : /%s/%s\n", ctx->stVencCfg[1].dstFilePath, ctx->stVencCfg[1].dstFileName);
    RK_PRINT("loop count            : %d\n", ctx->loopCountSet);
    RK_PRINT("ctx->enMode           : %d\n", ctx->enMode);
    RK_PRINT("enMemoryType          : %d\n", ctx->stChnAttr.stIspOpt.enMemoryType);
    RK_PRINT("ctx->aEntityName      : %s\n", ctx->stChnAttr.stIspOpt.aEntityName);
}
static RK_S32 create_venc(TEST_VI_CTX_S *ctx, RK_U32 u32Ch) {
    VENC_RECV_PIC_PARAM_S stRecvParam;

    RK_MPI_VENC_CreateChn(u32Ch, &ctx->stVencCfg[u32Ch].stAttr);
    RK_MPI_VENC_StartRecvFrame(u32Ch, &stRecvParam);
    RK_MPI_VENC_SetRcParam(u32Ch, &ctx->stVencCfg[u32Ch].stRcParam);

    return RK_SUCCESS;
}
TEST_VI_CTX_S *ctx = NULL;
void init_venc_cfg(TEST_VI_CTX_S *ctx, RK_U32 u32Ch, RK_CODEC_ID_E enType) {
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.enType = enType;
    ctx->stVencCfg[u32Ch].s32ChnId = u32Ch;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.enPixelFormat = RK_FMT_YUV422_YUYV;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32PicWidth = ctx->width;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32PicHeight = ctx->height;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32VirWidth = ctx->width;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32VirHeight = ctx->height;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32StreamBufCnt = 5;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32BufSize = ctx->width * ctx->height * 3 / 2;

    ctx->stVencCfg[u32Ch].stRcParam.s32FirstFrameStartQp = ctx->s32FirstFrameStartQp;
    ctx->stVencCfg[u32Ch].stRcParam.stParamH264.u32StepQp = ctx->u32StepQp;
    ctx->stVencCfg[u32Ch].stRcParam.stParamH264.u32MaxQp = ctx->u32MaxQp;
    ctx->stVencCfg[u32Ch].stRcParam.stParamH264.u32MinQp = ctx->u32MinQp;
}
// void board_sdk_api::_usb_check_camera_is_connected_thread()
// {
// 	sleep(3);
// 	while (1)
// 	{
// 		if (usb_camera_ != NULL)
// 		{
// 			struct stat buf;
// 			if (0 > stat("/dev/usbcam", &buf))
// 			{
// 				printf("/dev/usbcam file size = %d\n", buf.st_size);
// 				delete usb_camera_;
// 				usb_camera_ = NULL;
// 				_usb_video_stop();
// 				printf("%s:%d\n", __FILE__, __LINE__);
// 			}
// 		}
// 		sleep(1);
// 		printf("%s:%d %p\n", __FILE__, __LINE__,usb_camera_);
// 	}
// }
bool board_sdk_api::_usb_video_start()
{
	RK_S32 i = 1;
    RK_S32 s32Ret = RK_FAILURE;

	std::lock_guard<std::mutex> lock(usb_video_ctx_mutex_);
	if(!ctx)
    	ctx = reinterpret_cast<TEST_VI_CTX_S *>(malloc(sizeof(TEST_VI_CTX_S)));
	
    memset(ctx, 0, sizeof(TEST_VI_CTX_S));
#if DEBUG_USB_CAMERA_HPCLASS_20230614_
	char dbg_filename_buff[255];
	time_t currentTime;
	struct tm *timeInfo;
	char timeString[20];
	// 获取当前时间
	time(&currentTime);
	timeInfo = localtime(&currentTime);
	// 格式化时间为字符串
	strftime(timeString, sizeof(timeString), "%Y_%-m_%-d_%-H_%-M_%-S", timeInfo);
	sprintf(dbg_filename_buff, "/tmp/usb_cam_%s.h264", timeString);
	printf("%s\n", dbg_filename_buff);
	ctx->h264_fp = fopen(dbg_filename_buff, "wb+");
#endif

    char device_path_buffer[PATH_MAX] = {0};
	ctx->width = 640;
    ctx->height = 480;
#if defined(RV1126B)
    ctx->devId = 8;
    ctx->channelId = 3;

    ssize_t len = readlink(USBCAM_DEVICE_PATH, device_path_buffer, sizeof(device_path_buffer) - 1);
    if (-1 == len) {
        strcpy(device_path_buffer, USBCAM_DEVICE_PATH);
    }
#else
    ctx->devId = 0;
    ctx->channelId = 3;
    strcpy(device_path_buffer, USBCAM_DEVICE_PATH);
#endif
    ctx->pipeId = ctx->devId;
    ctx->loopCountSet = 1000;
    ctx->enMode = TEST_VI_MODE_BIND_VENC;
    ctx->stChnAttr.stIspOpt.u32BufCount = 3;
    ctx->stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
    ctx->stChnAttr.u32Depth = 2;
    ctx->aEntityName = device_path_buffer;
    ctx->stChnAttr.enPixelFormat = RK_FMT_YUV422_YUYV;
    ctx->stChnAttr.stFrameRate.s32SrcFrameRate = 15;
    ctx->stChnAttr.stFrameRate.s32DstFrameRate = 15;
    ctx->s32FirstFrameStartQp = 10;
    ctx->u32StepQp = 5 ;
    ctx->u32MaxQp = 30 ;
    ctx->u32MinQp = 10 ;

	//std::thread t3(&board_sdk_api::_usb_check_camera_is_connected_thread, this);
	RK_U32 u32DstCount = ((ctx->enMode == TEST_VI_MODE_BIND_VENC_MULTI) ? 2 : 1);
	 if (!ctx->width || !ctx->height) {
        //argparse_usage(&argparse);
        goto __FAILED3;
    }

    printf("test running enter ctx->aEntityName=%s!", ctx->aEntityName);
    if (ctx->aEntityName != RK_NULL)
        memcpy(ctx->stChnAttr.stIspOpt.aEntityName, ctx->aEntityName, strlen(ctx->aEntityName));

    mpi_vi_test_show_options(ctx);

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        goto __FAILED;
    }
#if defined(RV1126B)

#else
    /* vi init */
    // 0. get dev config status
    s32Ret = RK_MPI_VI_GetDevAttr(ctx->devId, &ctx->stDevAttr);
    if (s32Ret == RK_ERR_VI_NOT_CONFIG) {
        // 0-1.config dev
        s32Ret = RK_MPI_VI_SetDevAttr(ctx->devId, &ctx->stDevAttr);
        if (s32Ret != RK_SUCCESS) {
            printf("RK_MPI_VI_SetDevAttr %x", s32Ret);
            goto __FAILED1;
        }
    } else {
        printf("RK_MPI_VI_SetDevAttr already");
    }
    // 1.get  dev enable status
    s32Ret = RK_MPI_VI_GetDevIsEnable(ctx->devId);
    if (s32Ret != RK_SUCCESS) {
        // 1-2.enable dev
        s32Ret = RK_MPI_VI_EnableDev(ctx->devId);
        if (s32Ret != RK_SUCCESS) {
            printf("RK_MPI_VI_EnableDev %x", s32Ret);
            goto __FAILED1;
        }
        // 1-3.bind dev/pipe
        ctx->stBindPipe.u32Num = ctx->pipeId;
        ctx->stBindPipe.PipeId[0] = ctx->pipeId;
        s32Ret = RK_MPI_VI_SetDevBindPipe(ctx->devId, &ctx->stBindPipe);
        if (s32Ret != RK_SUCCESS) {
            printf("RK_MPI_VI_SetDevBindPipe %x", s32Ret);
            goto __FAILED2;
        }
    } else {
        printf("RK_MPI_VI_EnableDev already");
    }
#endif
    // 2.config channel
    ctx->stChnAttr.stSize.u32Width = ctx->width;
    ctx->stChnAttr.stSize.u32Height = ctx->height;
    s32Ret = RK_MPI_VI_SetChnAttr(ctx->pipeId, ctx->channelId, &ctx->stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        printf("RK_MPI_VI_SetChnAttr %x", s32Ret);
        goto __FAILED2;
    }
    // 3.enable channel
    s32Ret = RK_MPI_VI_EnableChn(ctx->pipeId, ctx->channelId);
    if (s32Ret != RK_SUCCESS) {
        printf("RK_MPI_VI_EnableChn %x", s32Ret);
        goto __FAILED2;
    }
    // // 4.save debug file
    // if (ctx->stDebugFile.bCfg) {
    //     s32Ret = RK_MPI_VI_ChnSaveFile(ctx->pipeId, ctx->channelId, &ctx->stDebugFile);
    //     RK_LOGD("RK_MPI_VI_ChnSaveFile %x", s32Ret);
    // }
	
	
    /* venc */
    // venc  init and create
    init_venc_cfg(ctx, i, RK_VIDEO_ID_AVC);
    s32Ret = create_venc(ctx, ctx->stVencCfg[i].s32ChnId);
    if (s32Ret != RK_SUCCESS) {
        printf("create %d ch venc failed", ctx->stVencCfg[i].s32ChnId);
        return s32Ret;
    }
     // bind vi to venc
    ctx->stSrcChn[i].enModId    = RK_ID_VI;
    ctx->stSrcChn[i].s32DevId   = ctx->devId;
    ctx->stSrcChn[i].s32ChnId   = ctx->channelId;

    ctx->stDestChn[i].enModId   = RK_ID_VENC;
#if defined(RV1126B)
    ctx->stDestChn[i].s32DevId  = 0;
#else
    ctx->stDestChn[i].s32DevId  = i;
#endif
    ctx->stDestChn[i].s32ChnId  = ctx->stVencCfg[i].s32ChnId;

    s32Ret = RK_MPI_SYS_Bind(&(ctx->stSrcChn[i]), &(ctx->stDestChn[i]));
    if (s32Ret != RK_SUCCESS) {
        printf("create %d ch venc failed", ctx->stVencCfg[i].s32ChnId);
        goto __FAILED2;
    }
    ctx->stFrame[i].pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));
	VENC_RC_PARAM_S pstRcParam;
    s32Ret =  RK_MPI_VENC_GetRcParam(ctx->stVencCfg[i].s32ChnId, &pstRcParam);
    printf("s32DeltIpQp is %x,%d\n\n",s32Ret,pstRcParam.stParamH264.s32DeltIpQp);
    printf("u32MaxQp is %x,%d\n\n",s32Ret,pstRcParam.stParamH264.u32MaxQp);
    printf("u32MaxIQp is %x,%d\n\n",s32Ret,pstRcParam.stParamH264.u32MaxIQp);
    printf("u32MinIQp is %x,%d\n\n",s32Ret,pstRcParam.stParamH264.u32MinIQp);
    printf("u32MinQp is %x,%d\n\n",s32Ret,pstRcParam.stParamH264.u32MinQp);
    printf("u32StepQp is %x,%d\n\n",s32Ret,pstRcParam.stParamH264.u32StepQp);
		 
#if TEST_WITH_FD
    ctx->stVencCfg[i].selectFd = RK_MPI_VENC_GetFd(ctx->stVencCfg[i].s32ChnId);
    printf("venc chn:%d, ctx->selectFd:%d ", ctx->stVencCfg[i].s32ChnId, ctx->stVencCfg[i].selectFd);
#endif
	return true;

__FAILED2:
	s32Ret = RK_MPI_VI_DisableDev(ctx->devId);
	printf("RK_MPI_VI_DisableDev %x", s32Ret);
__FAILED1:
//	for (i = 0; i < u32DstCount; i++)
	{
		if (ctx->stFrame[i].pstPack)
			free(ctx->stFrame[i].pstPack);
		if (ctx->stVencCfg[i].fp)
			fclose(ctx->stVencCfg[i].fp);
	}
__FAILED:
	printf("test running exit:%d", s32Ret);
	RK_MPI_SYS_Exit();
__FAILED3:
	if (ctx)
	{
		free(ctx);
		ctx = RK_NULL;
	}
	return false;
}

void board_sdk_api::_usb_video_stop()
{
	RK_S32 s32Ret = RK_FAILURE;
	RK_U32 i = 1;
	
	usb_stream_on_ = false;
	if (!ctx)
		return;
	std::lock_guard<std::mutex> lock(usb_video_ctx_mutex_);
	TEST_VI_CTX_S *ctx_del = ctx;
	ctx = NULL;
	s32Ret = RK_MPI_SYS_UnBind(&(ctx_del->stSrcChn[i]), &(ctx_del->stDestChn[i]));
	if (s32Ret != RK_SUCCESS)
	{
		printf("RK_MPI_SYS_UnBind fail %x\n", s32Ret);
	}

	// 5. disable one chn
	s32Ret = RK_MPI_VI_DisableChn(ctx_del->pipeId, ctx_del->channelId);
    if (s32Ret != RK_SUCCESS) {
        WriteLog(LOG_ERROR, "RK_MPI_VI_DisableChn fail %x", s32Ret);
        system("/etc/init.d/video-server restart &");
        return;
    }
#if DEBUG_USB_CAMERA_HPCLASS_20230614_

	if (ctx_del->h264_fp)
	{
		fclose(ctx_del->h264_fp);
	}
#endif

	s32Ret = RK_MPI_VENC_StopRecvFrame(ctx_del->stVencCfg[i].s32ChnId);
	if (s32Ret != RK_SUCCESS)
	{
		return;
	}
	printf("destroy enc chn:%d\n", ctx_del->stVencCfg[i].s32ChnId);
	s32Ret = RK_MPI_VENC_DestroyChn(ctx_del->stVencCfg[i].s32ChnId);
	if (s32Ret != RK_SUCCESS)
	{
		printf("RK_MPI_VDEC_DestroyChn fail %x", s32Ret);
	}

	// 6.disable dev(will diabled all chn)
	// s32Ret = RK_MPI_VI_DisableDev(ctx_del->devId);
	// printf("RK_MPI_VI_DisableDev %x\n", s32Ret);
	if (ctx_del->stFrame[i].pstPack)
		free(ctx_del->stFrame[i].pstPack);
	if (ctx_del->stVencCfg[i].fp)
		fclose(ctx_del->stVencCfg[i].fp);
	if (ctx_del)
		free(ctx_del);
	ctx_del = NULL;
}

bool board_sdk_api::_video_start()
{
	RK_U32 u32Width = config.hdmi_in.v_width;
	RK_U32 u32Height = config.hdmi_in.v_height;

    RK_U32 u32Gop = config.video[0].video_out.i_interval;
    RK_U32 u32BitRate = config.video[0].video_out.bitrate;
    RK_U32 u32BitType = config.video[0].video_out.bittype;
    RK_U32 DstFrameRateNum = config.video[0].video_out.frame_rate;
    RK_U32 u32Profile = config.video[0].video_out.encode_profile;
    RK_U32 u32EncodeType = config.video[0].video_out.encode_type;
	RK_U32 ret = -1;

	VI_DEV_ATTR_S stViDevAttr;
	VI_CHN_ATTR_S stViChnAttr;
	VENC_CHN_ATTR_S stChnAttr;
	VI_DEV_BIND_PIPE_S stBindPipe;
    VENC_RC_PARAM_S stRcParam;
	VENC_RECV_PIC_PARAM_S stRecvParam;

#if defined(RV1126B)
	int devId = 8;
	int pipeId = 8;
#else
	int devId = 0;
	int pipeId = 0;
#endif
	int chnId = 0;

	memset(&stViDevAttr, 0, sizeof(stViDevAttr));
	memset(&stBindPipe, 0, sizeof(stBindPipe));
#if defined(RV1126B)

#else
	ret = RK_MPI_VI_GetDevAttr(devId, &stViDevAttr);
	if (ret == RK_ERR_VI_NOT_CONFIG) {
		ret = RK_MPI_VI_SetDevAttr(devId, &stViDevAttr);
		if (ret != RK_SUCCESS) {
			return -1;
		}
	} else {
		printf("RK_MPI_VI_SetDevAttr already\n");
	}

	ret = RK_MPI_VI_GetDevIsEnable(devId);
	if (ret != RK_SUCCESS) {
		ret = RK_MPI_VI_EnableDev(devId);
		if (ret != RK_SUCCESS) {
			return -1;
		}
		// 1-3.bind dev/pipe
		stBindPipe.u32Num = pipeId;
		stBindPipe.PipeId[0] = pipeId;
		ret = RK_MPI_VI_SetDevBindPipe(devId, &stBindPipe);
		if (ret != RK_SUCCESS) {
			printf("RK_MPI_VI_SetDevBindPipe %x", ret);
			return -1;
		}

	} else {
		printf("RK_MPI_VI_EnableDev already\n");
	}
#endif
	memset(&stViChnAttr, 0, sizeof(VI_CHN_ATTR_S));
	strcpy(stViChnAttr.stIspOpt.aEntityName, "/dev/video0");
#if defined(RV1106)
	stViChnAttr.stIspOpt.u32BufCount = 3;
	//stViChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_MMAP;
	stViChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF; 
    stViChnAttr.enPixelFormat = RK_FMT_YUV422SP;
#elif defined(RV1126B)
	stViChnAttr.stIspOpt.u32BufCount = 3;
	stViChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
    stViChnAttr.enPixelFormat = RK_FMT_YUV420SP;
#else
	stViChnAttr.stIspOpt.u32BufCount = 8;
	stViChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_MMAP;
	/* stViChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF; */
    stViChnAttr.enPixelFormat = RK_FMT_YUV422SP;
#endif
	stViChnAttr.stSize.u32Width = u32Width;
	stViChnAttr.stSize.u32Height = u32Height;
	stViChnAttr.enCompressMode = COMPRESS_MODE_NONE;
	stViChnAttr.stIspOpt.bNoUseLibV4L2 = (RK_BOOL)1;
	/* stViChnAttr.stFrameRate.s32SrcFrameRate = fps; */
	/* stViChnAttr.stFrameRate.s32DstFrameRate = fps; */
	stViChnAttr.u32Depth = 1;// 0 bind 方式, 
	ret = RK_MPI_VI_SetChnAttr(devId, chnId, &stViChnAttr);
	if (ret != RK_SUCCESS) {
		return -1;
	}

	ret = RK_MPI_VI_EnableChn(devId, chnId);
	if (ret != RK_SUCCESS) {
		return -1;
	}

	memset(&stChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
	stChnAttr.stVencAttr.enType = RK_VIDEO_ID_AVC;

	// default VBR && h264
	if (u32BitType == 0 && u32EncodeType == 0)
		stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;

	if (u32BitType == 1 && u32EncodeType == 0)
		stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;

	if (u32BitType == 0 && u32EncodeType == 1)
		stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;

	if (u32BitType == 1 && u32EncodeType == 1)
		stChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;

	if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264CBR) {
		stChnAttr.stRcAttr.stH264Cbr.u32BitRate = u32BitRate;
		stChnAttr.stRcAttr.stH264Cbr.u32Gop = u32Gop;
		/* stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = DstFrameRateNum; */
		stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = -1;
		stChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
		stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = DstFrameRateNum;
		/* stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = -1; */
		stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
	} 

	if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR) {

		/* rk_common_rc.h
		RK_U32 u32BitRate;     // RW; Range:[2, 200000]kbps; average bitrate; default : u32VirWidth*u32VirHeight/8*30;
		RK_U32 u32MaxBitRate;  // RW; Range:[u32BitRate, 200000]kbps; max bitrate; default : u32BitRate*3/2;
		RK_U32 u32MinBitRate;  // RW; Range:[2, u32BitRate]kbps; min bitrate
		*/

		stChnAttr.stRcAttr.stH264Vbr.u32BitRate = u32BitRate;
		stChnAttr.stRcAttr.stH264Vbr.u32MaxBitRate = u32BitRate * 3 / 2;
		stChnAttr.stRcAttr.stH264Vbr.u32MinBitRate = u32BitRate / 2;
		stChnAttr.stRcAttr.stH264Vbr.u32Gop = u32Gop;
		/* stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateNum = DstFrameRateNum; */
		stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateNum = -1;
		stChnAttr.stRcAttr.stH264Vbr.u32SrcFrameRateDen = 1;
		stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateNum = DstFrameRateNum;
		/* stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateNum = -1; */
		stChnAttr.stRcAttr.stH264Vbr.fr32DstFrameRateDen = 1;
	}

	if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR) {
		stChnAttr.stRcAttr.stH265Cbr.u32BitRate = u32BitRate;
		stChnAttr.stRcAttr.stH265Cbr.u32Gop = u32Gop;
		stChnAttr.stVencAttr.enType = RK_VIDEO_ID_HEVC;
		/* stChnAttr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = DstFrameRateNum; */
		stChnAttr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = -1;
		stChnAttr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = 1;
		stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = DstFrameRateNum;
		/* stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = -1; */
		stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = 1;
	}

	if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR) {
		stChnAttr.stRcAttr.stH265Vbr.u32BitRate = u32BitRate;
		stChnAttr.stRcAttr.stH265Vbr.u32MaxBitRate = u32BitRate * 3 / 2;
		stChnAttr.stRcAttr.stH265Vbr.u32MinBitRate = u32BitRate / 2;
		stChnAttr.stRcAttr.stH265Vbr.u32Gop = u32Gop;
		stChnAttr.stVencAttr.enType = RK_VIDEO_ID_HEVC;
		/* stChnAttr.stRcAttr.stH265Vbr.u32SrcFrameRateNum = DstFrameRateNum; */
		stChnAttr.stRcAttr.stH265Vbr.u32SrcFrameRateNum = -1;
		stChnAttr.stRcAttr.stH265Vbr.u32SrcFrameRateDen = 1;
		stChnAttr.stRcAttr.stH265Vbr.fr32DstFrameRateNum = DstFrameRateNum;
		/* stChnAttr.stRcAttr.stH265Vbr.fr32DstFrameRateNum = -1; */
		stChnAttr.stRcAttr.stH265Vbr.fr32DstFrameRateDen = 1;
	}


	/* stChnAttr.stVencAttr.u32Profile = H264E_PROFILE_HIGH; */
	stChnAttr.stVencAttr.u32Profile = u32Profile;
#if defined(RV1106)
	stChnAttr.stVencAttr.enPixelFormat = RK_FMT_YUV422SP;
#else
	stChnAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
#endif
	stChnAttr.stVencAttr.u32PicWidth = u32Width;
	stChnAttr.stVencAttr.u32PicHeight = u32Height;
	stChnAttr.stVencAttr.u32VirWidth = u32Width;
	stChnAttr.stVencAttr.u32VirHeight = u32Height;
	stChnAttr.stVencAttr.u32StreamBufCnt = 1;
	stChnAttr.stVencAttr.u32BufSize = u32Width * u32Height * 3 / 2;
	stChnAttr.stVencAttr.enMirror = MIRROR_NONE;

	ret = RK_MPI_VENC_CreateChn(0, &stChnAttr);
	printf("RK_MPI_VENC_CreateChn ret = %d\n", ret);

	if (ret != RK_SUCCESS) {
		return -1;	
	}

	// rv1126b 必须设置Qp参数(且要调低 MinQp MinIQp FrmMinQp FrmMinIQp, 应该主要是FrmMinQp FrmMinIQp影响最大)，否则码率达不到设置值, 
	// 另外有个一定要注意的地方
	// 设置了 min 一定要设置对应的max 否则,这个max会因为随机值导致设置码率时 硬件编码器画面直接不更新(这里很坑查了很久 要注意)

#if defined(RV1126B) 
	memset(&stRcParam, 0, sizeof(VENC_RC_PARAM_S));

	// stRcParam.s32FirstFrameStartQp = 26;
	// stRcParam.stParamH264.u32StepQp = 8;
	// stRcParam.stParamH264.s32DeltIpQp = 2;

	// stRcParam.stParamH264.u32MaxQp = 51;
	stRcParam.stParamH264.u32MinQp = 10;
	// stRcParam.stParamH265.u32MaxQp = 51;
	stRcParam.stParamH265.u32MinQp = 10;

	stRcParam.stParamH264.u32MinIQp = 10;
	// stRcParam.stParamH264.u32MaxIQp = 51;
	stRcParam.stParamH265.u32MinIQp = 10;
	// stRcParam.stParamH265.u32MaxIQp = 51;


	stRcParam.stParamH264.u32FrmMinQp = 10;
	// stRcParam.stParamH264.u32FrmMaxQp = 42;
	stRcParam.stParamH265.u32FrmMinQp = 10;
	// stRcParam.stParamH265.u32FrmMaxQp = 42;

	stRcParam.stParamH264.u32FrmMinIQp = 10;
	// stRcParam.stParamH264.u32FrmMaxIQp = 42;
	stRcParam.stParamH265.u32FrmMinIQp = 10;
	// stRcParam.stParamH265.u32FrmMaxIQp = 42;
	// stRcParam.stParamH264.u32FrmMinQp = 27;
	// stRcParam.stParamH264.u32FrmMaxQp = 42;

	stRcParam.stParamH264.s32MaxReEncodeTimes = 1;
	RK_MPI_VENC_SetRcParam(0, &stRcParam);
#endif

	memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));
	stRecvParam.s32RecvPicNum = -1;
	RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);

	if (u32EncodeType == 0) {
		VENC_H264_VUI_S stH264Vui;
		RK_MPI_VENC_GetH264Vui(0, &stH264Vui);
		stH264Vui.stVuiVideoSignal.video_full_range_flag = 0;
		RK_MPI_VENC_SetH264Vui(0, &stH264Vui);
		RK_MPI_VENC_GetH264Vui(0, &stH264Vui);
	} 

	if (u32EncodeType == 1) {
		VENC_H265_VUI_S stH265Vui;
		RK_MPI_VENC_GetH265Vui(0, &stH265Vui);
		stH265Vui.stVuiVideoSignal.video_full_range_flag = 0;
		RK_MPI_VENC_SetH265Vui(0, &stH265Vui);
		RK_MPI_VENC_GetH265Vui(0, &stH265Vui);
	} 

    return true;
}

void board_sdk_api::_on_resolution_change(uint16_t width, uint16_t height, uint16_t frame_rate)
{
	// printf("[sunchao]  board_sdk_api::_video_stop ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 333 \n");
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	exit(0);
}

void board_sdk_api::_video_stop()
{
	RK_MPI_VENC_StopRecvFrame(0);
	RK_MPI_VENC_DestroyChn(0);
#if defined(RV1126B)
	RK_MPI_VI_DisableChn(8, 0);
#else
	RK_MPI_VI_DisableChn(0, 0);
	RK_MPI_VI_DisableDev(0);
#endif
	return;
}

bool board_sdk_api::force_i_frame(stream_channel_num channel)
{
	RK_U32 ret = -1;
	for (int i = 0; i < 10; i ++) {
		ret = RK_MPI_VENC_RequestIDR(0, RK_FALSE);
		if (ret ==  RK_SUCCESS) {
			WriteLog(LOG_DEBUG, "[videoserver] board_sdk_api::force_i_frame RK_MPI_VENC_RequestIDR success");
			break;
		}
		WriteLog(LOG_ERROR, "[videoserver] board_sdk_api::force_i_frame RK_MPI_VENC_RequestIDR failed [cnt:%d, err:%x]", i, ret);
		usleep(1000*1000);
	}
	return (ret == RK_SUCCESS) ? true : false;
}

bool board_sdk_api::get_hdmi_resolution(int* width, int* height, int* src_frame, int* audio_sampling, int *scan_mode)
{
	*width  = config.hdmi_in.v_width;
	*height = config.hdmi_in.v_height;
    *src_frame = 25;

    return true;
}

int board_sdk_api::_usb_get_video_data(stream_channel_num channel, void **frame_addr, size_t *frame_size, size_t *pack_count)
{
	RK_S32 loopCount = 0;
	void *pData = RK_NULL;
	RK_S32 s32Ret = RK_FAILURE;
	RK_U32 i = 1;
	int wait_time = 200;
	if (!ctx)
	{
		*pack_count = 0;
		return 0;
	}
	std::lock_guard<std::mutex> lock(usb_video_ctx_mutex_);
	//printf("\033[042m [huangpeng] ch ID => %d :%d \033[0m\n", ctx->stVencCfg[i].s32ChnId, __LINE__);
	s32Ret = RK_MPI_VENC_GetStream(ctx->stVencCfg[i].s32ChnId, &ctx->stFrame[i], wait_time);
	if (s32Ret == RK_SUCCESS)
	{
		// if (ctx->stVencCfg[i].bOutDebugCfg) {
		pData = RK_MPI_MB_Handle2VirAddr(ctx->stFrame[i].pstPack->pMbBlk);

		/// for sunlogin
		memcpy(usb_video_stream_buf_, pData, ctx->stFrame[i].pstPack->u32Len);
#if DEBUG_USB_CAMERA_HPCLASS_20230614_
		if (ctx->h264_fp)
		{
			fwrite(usb_video_stream_buf_, 1, ctx->stFrame[i].pstPack->u32Len, ctx->h264_fp);
		}
#endif
		if (usb_video_stream_buf_[4] == 0x67)
		{
			*pack_count = 4;
			char target_27[5] = {0x00, 0x00, 0x00, 0x01, 0x67};
			char *p_27 = board_sdk_tool::find_taeget(usb_video_stream_buf_, ctx->stFrame[i].pstPack->u32Len, target_27, 5);
			char target_28[5] = {0x00, 0x00, 0x00, 0x01, 0x68};
			char *p_28 = board_sdk_tool::find_taeget(usb_video_stream_buf_, ctx->stFrame[i].pstPack->u32Len, target_28, 5);
			char target_25[5] = {0x00, 0x00, 0x00, 0x01, 0x65};
			char *p_25 = board_sdk_tool::find_taeget(usb_video_stream_buf_, ctx->stFrame[i].pstPack->u32Len, target_25, 5);

			*(frame_addr + 0) = (void *)p_27;
			*(frame_size + 0) = p_28 - p_27;

			*(frame_addr + 1) = (void *)p_28;
			*(frame_size + 1) = p_25 - p_28;

			*(frame_addr + 2) = (void *)p_sei_;
			*(frame_size + 2) = 55;

			*(frame_addr + 3) = (void *)p_25;
			*(frame_size + 3) = ctx->stFrame[i].pstPack->u32Len - (p_25 - p_27);

			WriteLog(LOG_DEBUG,"%x  %x  %x",p_27, p_28, p_25);
			WriteLog(LOG_DEBUG,"get frame ok 1 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 0), p_27[0], p_27[1], p_27[2], p_27[3], p_27[4]);
			WriteLog(LOG_DEBUG,"get frame ok 2 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 1), p_28[0], p_28[1], p_28[2], p_28[3], p_28[4]);
			WriteLog(LOG_DEBUG,"get frame ok 3 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 2), p_sei_[0], p_sei_[1], p_sei_[2], p_sei_[3], p_sei_[4]);
			WriteLog(LOG_DEBUG,"get frame ok 4 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 3), p_25[0], p_25[1], p_25[2], p_25[3], p_25[4]);
		}
		else
		{
			*pack_count = 1;
			*(frame_addr) = (void *)(usb_video_stream_buf_);
			*(frame_size) = ctx->stFrame[i].pstPack->u32Len;
		}
		s32Ret = RK_MPI_VENC_ReleaseStream(ctx->stVencCfg[i].s32ChnId, &ctx->stFrame[i]);
		if (s32Ret != RK_SUCCESS)
		{
			printf("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
		}
		loopCount++;
	}
	else
	{
		printf("RK_MPI_VI_GetChnFrame fail %x", s32Ret);
	}
	return s32Ret;
}

int board_sdk_api::_get_video_data(stream_channel_num channel, char *frame_tmp_buf, void **frame_addr, size_t *frame_size, size_t *pack_count, unsigned long *frame_pts)
{
	struct timeval tv1, tv2;
	static struct timeval tv1_1, tv2_2;
    gettimeofday(&tv1,NULL);

    int evl_time = 1000/config.video[0].video_out.frame_rate;

	/* return 0; */
#if defined(RV1126B)
	int viPipe = 8;
#else
	int viPipe = 0;
#endif
	int viChn = 0, vencChn = 0;
	int wait_time = 100;
	int len = 0;
	unsigned long pts = 0;
	void * data = NULL;
	/* int wait_time = 10; */

///  get and send frame from vi to venc
	VIDEO_FRAME_INFO_S stViFrame;
	VENC_STREAM_S stVencStream;
	int s32Ret = -1;

	memset(&stViFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

	s32Ret = RK_MPI_VI_GetChnFrame(viPipe, viChn, &stViFrame, wait_time);
    if(s32Ret < 0) {
        WriteLog(LOG_ERROR, "RK_MPI_VI_GetChnFrame failed:%d", s32Ret);
        RK_MPI_VI_ReleaseChnFrame(viPipe, viChn, &stViFrame);
        return -1;
    }

#define ENABLE_DEBUG_VI 0
#if ENABLE_DEBUG_VI
	data = RK_MPI_MB_Handle2VirAddr(stViFrame.stVFrame.pMbBlk);
	static FILE * fp = fopen("/tmp/video.yuv","wb+");
	static int count = 2;
	if (fp && (count-- > 0)) {
		fwrite(data, 1, RK_MPI_MB_GetSize(stViFrame.stVFrame.pMbBlk), fp);
		fflush(fp);
	}
#endif

	s32Ret = RK_MPI_VENC_SendFrame(0, (VIDEO_FRAME_INFO_S *)&stViFrame, wait_time);
    if(s32Ret < 0) {
        WriteLog(LOG_ERROR, "RK_MPI_VENC_SendFrame failed:%d", s32Ret);
        RK_MPI_VI_ReleaseChnFrame(viPipe, viChn, &stViFrame);
        return -1;
    }

	RK_MPI_VI_ReleaseChnFrame(viPipe, viChn, &stViFrame);

	stVencStream.pstPack = (VENC_PACK_S *) calloc(1, sizeof(VENC_PACK_S));
	s32Ret = RK_MPI_VENC_GetStream(vencChn, &stVencStream, wait_time);//当设定帧率时这里超时
	if(s32Ret < 0) {
		WriteLog(LOG_ERROR, "venc getstream failed");
		if(stVencStream.pstPack) {
			free(stVencStream.pstPack);
		}
		return -1;
	}

	data = RK_MPI_MB_Handle2VirAddr(stVencStream.pstPack->pMbBlk);
	if(!data) {
		WriteLog(LOG_ERROR, "get video data is null, release mem");
		if(stVencStream.pstPack) {
			free(stVencStream.pstPack);
		}
		return -1;
	}

	RK_MPI_SYS_MmzFlushCache(stVencStream.pstPack->pMbBlk, RK_TRUE);

	len = stVencStream.pstPack->u32Len;
	pts = stVencStream.pstPack->u64PTS;

/// for sunlogin
	// memcpy(video_stream_buf_ , data, len);
    // 复制帧数据到传入的ringbuffer槽位，并保存时间戳信息
    memcpy(frame_tmp_buf , data, len);
    *frame_pts = pts;

#define ENABLE_DEBUG_VENC 0
#if ENABLE_DEBUG_VENC
	static FILE * fp_venc = fopen("/tmp/video.h26x","wb+");
	if (fp_venc) {
		fwrite(data, 1, len, fp_venc);
		fflush(fp_venc);
	}
#endif

	if ( frame_tmp_buf[4] == 0x67 )
	{
		*pack_count = 4;
		char target_27[5] = { 0x00, 0x00, 0x00, 0x01, 0x67 };
		char *p_27 = board_sdk_tool::find_taeget(frame_tmp_buf, len, target_27, 5);
		char target_28[5] = { 0x00, 0x00, 0x00, 0x01, 0x68 };
		char *p_28 = board_sdk_tool::find_taeget(frame_tmp_buf, len, target_28, 5);
		char target_25[5] = { 0x00, 0x00, 0x00, 0x01, 0x65 };
		char *p_25 = board_sdk_tool::find_taeget(frame_tmp_buf, len, target_25, 5);

		*(frame_addr + 0) = (void *) p_27;
		*(frame_size + 0) = p_28 - p_27;

		*(frame_addr + 1) = (void *) p_28;
		*(frame_size + 1) = p_25 - p_28;

		*(frame_addr + 2) = (void *) p_sei_;
		*(frame_size + 2) = 55;

		*(frame_addr + 3) = (void *) p_25;
		*(frame_size + 3) = len - (p_25 - p_27);

		// WriteLog(LOG_DEBUG,"%x  %x  %x",p_27, p_28, p_25);
		// WriteLog(LOG_DEBUG,"get frame ok 1 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 0), p_27[0], p_27[1], p_27[2], p_27[3], p_27[4]);
		// WriteLog(LOG_DEBUG,"get frame ok 2 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 1), p_28[0], p_28[1], p_28[2], p_28[3], p_28[4]);
		// WriteLog(LOG_DEBUG,"get frame ok 3 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 2), p_sei_[0], p_sei_[1], p_sei_[2], p_sei_[3], p_sei_[4]);
		// WriteLog(LOG_DEBUG,"get frame ok 4 lem %06d  %02x %02x %02x %02x %02x", *(frame_size + 3), p_25[0], p_25[1], p_25[2], p_25[3], p_25[4]);

	}else{
		*pack_count = 1;
		*(frame_addr) = (void *)(frame_tmp_buf);
		*(frame_size) = len;
	}

	RK_MPI_VENC_ReleaseStream(vencChn, &stVencStream);
	if(stVencStream.pstPack) {
		free(stVencStream.pstPack);
	}

_exit:
	gettimeofday(&tv2,NULL);

    /* int sleep_time = evl_time - (tv2.tv_sec*1000 + tv2.tv_usec/1000 - (tv1.tv_sec*1000 + tv1.tv_usec/1000)); */
    /* if ( sleep_time > 0 && sleep_time <= evl_time ) */
    /* { */
    /*     std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time)); */
    /* } */

	return 0;
}



bool board_sdk_api::_audio_start()
{
#if 0
	const char * srcFilePath        = RK_NULL;
	const char * dstFilePath        = RK_NULL;

	int s32ChnNum          = 1;
	int s32PeriodCount     = 1;
	int s32PeriodSize      = 480;
	const char * chCardName         = RK_NULL;
	int s32DevId           = 0;

	RK_U32 s32DeviceSampleRate = 48000;
	RK_U32 s32DeviceChannel = 1;
	RK_U32 s32ChnIndex = 0;
	/* RK_U32 u32FrameCnt = 480; */


	AUDIO_DEV aiDevId = s32DevId;
	AUDIO_SOUND_MODE_E soundMode;
	AIO_ATTR_S aiAttr;
	RK_S32 result;

	memset(&aiAttr, 0, sizeof(AIO_ATTR_S));

	if (chCardName) {
		snprintf(reinterpret_cast<char *>(aiAttr.u8CardName),sizeof(aiAttr.u8CardName), "%s", chCardName);
	}

	aiAttr.soundCard.channels = s32DeviceChannel;
	aiAttr.soundCard.sampleRate = s32DeviceSampleRate;
	aiAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;

	aiAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	aiAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)s32DeviceSampleRate;

	aiAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	aiAttr.u32FrmNum = s32PeriodCount;
	aiAttr.u32PtNumPerFrm = s32PeriodSize;
	aiAttr.u32EXFlag = 0;
	aiAttr.u32ChnCnt = 1;

	result = RK_MPI_AI_SetPubAttr(s32DevId, &aiAttr);
	if(result != 0) {
		goto _failed;
	}

	result = RK_MPI_AI_Enable(s32DevId);
	if(result != 0) {
		goto _failed;
	}

	result = RK_MPI_AI_EnableChn(s32DevId, s32ChnIndex);
	if(result != 0) {
		goto _failed;
	}

	return true;
_failed:
	return false;
#else
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
#if defined(RV1106)
	unsigned int channels = 2;
	unsigned int rate = 48000;
	unsigned int buffer_time = 85334;
#else
	unsigned int channels = 1;
	unsigned int rate = 48000;
	unsigned int buffer_time = 40 * 1000;
#endif
	if ( _alsa_capture == NULL)
	{
		_alsa_capture = new Calsa_wrapper(SND_PCM_STREAM_CAPTURE);
		_alsa_capture->init(rate, channels, format, buffer_time);
		_alsa_param = _alsa_capture->get_alsa_params();
	}
    return true;
#endif
}


void board_sdk_api::_audio_stop()
{
	// set_audio_codec_power(0, 2, "0");
	/* RK_MPI_AI_DisableChn(0, 0); */
}

#define TEST_FILE 0

int board_sdk_api::_get_audio_data(stream_channel_num channel, void **frame_addr, size_t *frame_size, size_t *pack_count)
{
#if 0
	AUDIO_FRAME_S frame;
	RK_S32 result = 0;

	result = RK_MPI_AI_GetFrame(0,0,&frame, RK_NULL, -1);
	if(result != 0) {
		return -1;
	}

	void * data = RK_MPI_MB_Handle2VirAddr(frame.pMbBlk);
	RK_U32 len = RK_MPI_MB_GetSize(frame.pMbBlk);
	WriteLog(LOG_INFO, "data = %p ,len = %d", data, len);
	RK_MPI_AI_ReleaseFrame(0,0, &frame, RK_NULL);
    
#else
	int len = 0;
	if ( _alsa_capture != NULL){
		len = _alsa_capture->pcm_read((unsigned char *)audio_stream_buf_, _alsa_param->chunk_size);
		//WriteLog(LOG_INFO, "alsa data = %p ,len = %d, chunk_size: %d", audio_stream_buf_,len , _alsa_param->chunk_size);
		if (len == _alsa_param->chunk_size) {
			*frame_addr = audio_stream_buf_;
			*frame_size = len;
			*pack_count = 1;
		}
		else {
			return -1;
		}
	}
#endif

#if TEST_FILE
	static FILE * fp = NULL;
	if(!fp) {
		fp = fopen("/tmp/test.pcm", "a+");
	}
	if(fp) {
		fwrite(*frame_addr, *frame_size , 2, fp);
	}
#endif

	return 0;
}

bool board_sdk_api::_audio_MIC_start()
{
#if 0
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	unsigned int channels = 1;
	unsigned int rate = 48000;
	unsigned int buffer_time = 40 * 1000;
	WriteLog(LOG_INFO, "_audio_MIC_start\n\n\n");
	if (_alsa_MIC_capture == NULL) {
		_alsa_MIC_capture = new Calsa_wrapper(SND_PCM_STREAM_CAPTURE);
		_alsa_MIC_capture->init("plughw:1,0", rate, channels, format, buffer_time);
		_alsa_MIC_param = _alsa_MIC_capture->get_alsa_params();
	}
#endif

	return false;
}

void board_sdk_api::_audio_MIC_stop()
{
	// set_audio_codec_power(0, 2, "0");
	/* RK_MPI_AI_DisableChn(0, 0); */
}

#define TEST_FILE_MIC 0
int board_sdk_api::_get_audio_MIC_data(stream_channel_num channel, void **frame_addr, size_t *frame_size, size_t *pack_count)
{

	int len = 0;
	if ( _alsa_MIC_capture != NULL){
		len = _alsa_MIC_capture->pcm_read((unsigned char *)audio_MIC_stream_buf_, _alsa_MIC_param->chunk_size);
		//WriteLog(LOG_INFO, "MIC alsa data = %p ,len = %d, chunk_size: %d", audio_MIC_stream_buf_,len , _alsa_MIC_param->chunk_size);
		if (len == _alsa_MIC_param->chunk_size) {
			*frame_addr = audio_MIC_stream_buf_;
			*frame_size = len;
			*pack_count = 1;
		}
		else {
			return -1;
		}
	}

#if TEST_FILE_MIC
	static FILE * fp = NULL;
	if(!fp) {
		fp = fopen("/tmp/test_mic.pcm", "a+");
	}
	if(fp) {
		fwrite(*frame_addr, *frame_size , 2, fp);
	}
#endif

	return 0;
}
