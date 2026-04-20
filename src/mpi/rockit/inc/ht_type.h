/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_type.h                                          *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-9                                           *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef __HT_TYPE_H
#define __HT_TYPE_H

#include <stdint.h>

typedef unsigned char           HT_UCHAR;
typedef uint8_t                 HT_U8;
typedef uint16_t                HT_U16;
typedef uint32_t                HT_U32;
typedef uint32_t                HT_UL;
typedef uintptr_t               HT_UINTPTR_T;

typedef char                    HT_CHAR;
typedef int8_t                  HT_S8;
typedef int16_t                 HT_S16;
typedef int32_t                 HT_S32;
typedef int32_t                 HT_SL;

typedef float                   HT_FLOAT;
typedef double                  HT_DOUBLE;

typedef uint64_t                HT_U64;
typedef int64_t                 HT_S64;

typedef uint32_t                HT_SIZE_T;
typedef uint32_t                HT_LENGTH_T;

typedef void *            		HT_HANDLE;
typedef void 					HT_VOID;

typedef HT_U32 HT_VO_INTF_TYPE;

typedef enum {
    HT_FALSE = 0,
    HT_TRUE  = 1,
} HT_BOOL;

typedef HT_S32 (*cb_venc_get_frame)(void *user, void *data, HT_S32 len, HT_U64 pts);
typedef HT_S32 (*cb_tde_get_frame)(void *user, void *data, HT_S32 len);

#define HT_COLOR_RGB_RED      0xFF0000
#define HT_COLOR_RGB_GREEN    0x00FF00
#define HT_COLOR_RGB_BLUE     0x0000FF
#define HT_COLOR_RGB_BLACK    0x000000
#define HT_COLOR_RGB_YELLOW   0xFFFF00
#define HT_COLOR_RGB_CYN      0x00ffff
#define HT_COLOR_RGB_WHITE    0xffffff

//#define HT_VIDEO_FMT_MASK     0x000f0000
//#define HT_VIDEO_FMT_YUV      0x00000000
//#define HT_VIDEO_FMT_RGB      0x00010000
//#define HT_VIDEO_FMT_BAYER    0X00020000

#define HT_VO_INTF_CVBS       (0x01L << 0)
#define HT_VO_INTF_VGA        (0x01L << 1)
#define HT_VO_INTF_BT656      (0x01L << 2)
#define HT_VO_INTF_BT1120     (0x01L << 3)
#define HT_VO_INTF_HDMI       (0x01L << 4)
#define HT_VO_INTF_RGB_6BIT   (0x01L << 5)
#define HT_VO_INTF_RGB_8BIT   (0x01L << 6)
#define HT_VO_INTF_RGB_16BIT  (0x01L << 7)
#define HT_VO_INTF_RGB_18BIT  (0x01L << 8)
#define HT_VO_INTF_RGB_24BIT  (0x01L << 9)
#define HT_VO_INTF_MIPI       (0x01L << 10)
#define HT_VO_INTF_MIPI_SLAVE (0x01L << 11)
#define HT_VO_INTF_HDMI1      (0x01L << 12)
#define HT_VO_INTF_DP         (0x01L << 15)
#define HT_VO_INTF_DP1        (0x01L << 16)

typedef enum htCODEC_ID {
	HT_VIDEO_AUTO,
	HT_VIDEO_ID_H264,
	HT_VIDEO_ID_H265,
	HT_VIDEO_ID_VP9,
	HT_VIDEO_ID_JPEG,

	HT_VIDEO_BUTT
	
} HT_CODEC_ID;
	
typedef enum htVO_LAYER_MODE {
    HT_VO_LAYER_MODE_CURSOR = 0,
    HT_VO_LAYER_MODE_GRAPHIC = 1,
    HT_VO_LAYER_MODE_VIDEO = 2,
    HT_VO_LAYER_MODE_BUTT
} HT_VO_LAYER_MODE;


typedef enum htPIXEL_FORMAT {
    HT_FMT_YUV420SP         = 0,        /* YYYY... UV...            */
    HT_FMT_YUV420SP_10BIT,
    HT_FMT_YUV422SP,                                   /* YYYY... UVUV...          */
    HT_FMT_YUV422SP_10BIT,                             ///< Not part of ABI
    HT_FMT_YUV420P,                                    /* YYYY... UUUU... VVVV     */
    HT_FMT_YUV420P_VU,                                 /* YYYY... VVVV... UUUU     */
    HT_FMT_YUV420SP_VU,                                /* YYYY... VUVUVU...        */
    HT_FMT_YUV422P,                                    /* YYYY... UUUU... VVVV     */
    HT_FMT_YUV422SP_VU,                                /* YYYY... VUVUVU...        */
    HT_FMT_YUV422_YUYV,                                /* YUYVYUYV...              */
    HT_FMT_YUV422_UYVY,                                /* UYVYUYVY...              */
    HT_FMT_YUV400SP,                                   /* YYYY...                  */
    HT_FMT_YUV440SP,                                   /* YYYY... UVUV...          */
    HT_FMT_YUV411SP,                                   /* YYYY... UV...            */
    HT_FMT_YUV444,                                     /* YUVYUVYUV...             */
    HT_FMT_YUV444SP,                                   /* YYYY... UVUVUVUV...      */
    HT_FMT_YUV422_YVYU,                                /* YVYUYVYU...              */
    HT_FMT_YUV422_VYUY,                                /* VYUYVYUY...              */
    HT_FMT_YUV_BUTT,

    HT_FMT_RGB565,         								/* 16-bit RGB               */
    HT_FMT_BGR565,                                     /* 16-bit RGB               */
    HT_FMT_RGB555,                                     /* 15-bit RGB               */
    HT_FMT_BGR555,                                     /* 15-bit RGB               */
    HT_FMT_RGB444,                                     /* 12-bit RGB               */
    HT_FMT_BGR444,                                     /* 12-bit RGB               */
    HT_FMT_RGB888,                                     /* 24-bit RGB               */
    HT_FMT_BGR888,                                     /* 24-bit RGB               */
    HT_FMT_RGB101010,                                  /* 30-bit RGB               */
    HT_FMT_BGR101010,                                  /* 30-bit RGB               */
    HT_FMT_ARGB1555,                                   /* 16-bit RGB               */
    HT_FMT_ABGR1555,                                   /* 16-bit RGB               */
    HT_FMT_ARGB4444,                                   /* 16-bit RGB               */
    HT_FMT_ABGR4444,                                   /* 16-bit RGB               */
    HT_FMT_ARGB8565,                                   /* 24-bit RGB               */
    HT_FMT_ABGR8565,                                   /* 24-bit RGB               */
    HT_FMT_ARGB8888,                                   /* 32-bit RGB               */
    HT_FMT_ABGR8888,                                   /* 32-bit RGB               */
    HT_FMT_BGRA8888,                                   /* 32-bit RGB               */
    HT_FMT_RGBA8888,                                   /* 32-bit RGB               */
    HT_FMT_RGBA5551,                                   /* 16-bit RGB               */
    HT_FMT_BGRA5551,                                   /* 16-bit RGB               */
    HT_FMT_BGRA4444,                                   /* 16-bit RGB               */
    HT_FMT_RGB_BUTT,

    HT_FMT_RGB_BAYER_SBGGR_8BPP,  						/* 8-bit raw                */
    HT_FMT_RGB_BAYER_SGBRG_8BPP,                       /* 8-bit raw                */
    HT_FMT_RGB_BAYER_SGRBG_8BPP,                       /* 8-bit raw                */
    HT_FMT_RGB_BAYER_SRGGB_8BPP,                       /* 8-bit raw                */
    HT_FMT_RGB_BAYER_SBGGR_10BPP,                      /* 10-bit raw               */
    HT_FMT_RGB_BAYER_SGBRG_10BPP,                      /* 10-bit raw               */
    HT_FMT_RGB_BAYER_SGRBG_10BPP,                      /* 10-bit raw               */
    HT_FMT_RGB_BAYER_SRGGB_10BPP,                      /* 10-bit raw               */
    HT_FMT_RGB_BAYER_SBGGR_12BPP,                      /* 12-bit raw               */
    HT_FMT_RGB_BAYER_SGBRG_12BPP,                      /* 12-bit raw               */
    HT_FMT_RGB_BAYER_SGRBG_12BPP,                      /* 12-bit raw               */
    HT_FMT_RGB_BAYER_SRGGB_12BPP,                      /* 12-bit raw               */
    HT_FMT_RGB_BAYER_14BPP,                            /* 14-bit raw               */
    HT_FMT_RGB_BAYER_SBGGR_16BPP,                      /* 16-bit raw               */
    HT_FMT_RGB_BAYER_BUTT,
    HT_FMT_BUTT            = HT_FMT_RGB_BAYER_BUTT,
} HT_PIXEL_FORMAT;

typedef enum htMOD_ID {
    HT_ID_CMPI    = 0,
    HT_ID_MB      = 1,
    HT_ID_SYS     = 2,
    HT_ID_RGN     = 3,
    HT_ID_VENC    = 4,
    HT_ID_VDEC    = 5,
    HT_ID_VPSS    = 6,
    HT_ID_VGS     = 7,
    HT_ID_VI      = 8,
    HT_ID_VO      = 9,
    HT_ID_AI      = 10,
    HT_ID_AO      = 11,
    HT_ID_AENC    = 12,
    HT_ID_ADEC    = 13,
    HT_ID_TDE     = 14,
    HT_ID_ISP     = 15,
    HT_ID_WBC     = 16,
    HT_ID_AVS     = 17,

	//	do not remove HT_ID_BUTT
    HT_ID_BUTT,
} MOD_ID;

typedef enum htVO_INTF_SYNC {
    HT_VO_OUTPUT_PAL = 0, /* PAL standard */
    HT_VO_OUTPUT_NTSC, /* NTSC standard */

    HT_VO_OUTPUT_1080P24, /* 1920 x 1080 at 24 Hz. */
    HT_VO_OUTPUT_1080P25, /* 1920 x 1080 at 25 Hz. */
    HT_VO_OUTPUT_1080P30, /* 1920 x 1080 at 30 Hz. */

    HT_VO_OUTPUT_720P50, /* 1280 x  720 at 50 Hz. */
    HT_VO_OUTPUT_720P60, /* 1280 x  720 at 60 Hz. */

    HT_VO_OUTPUT_1080I50, /* 1920 x 1080 at 50 Hz, interlace. */
    HT_VO_OUTPUT_1080I60, /* 1920 x 1080 at 60 Hz, interlace. */
    HT_VO_OUTPUT_1080P50, /* 1920 x 1080 at 50 Hz. */
    HT_VO_OUTPUT_1080P60, /* 1920 x 1080 at 60 Hz. */

    HT_VO_OUTPUT_576P50, /* 720  x  576 at 50 Hz. */
    HT_VO_OUTPUT_480P60, /* 720  x  480 at 60 Hz. */

    HT_VO_OUTPUT_800x600_60, /* VESA 800 x 600 at 60 Hz (non-interlaced) */
    HT_VO_OUTPUT_1024x768_60, /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    HT_VO_OUTPUT_1280x1024_60, /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    HT_VO_OUTPUT_1366x768_60, /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    HT_VO_OUTPUT_1440x900_60, /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    HT_VO_OUTPUT_1280x800_60, /* 1280*800@60Hz VGA@60Hz */
    HT_VO_OUTPUT_1600x1200_60, /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
    HT_VO_OUTPUT_1680x1050_60, /* VESA 1680 x 1050 at 60 Hz (non-interlaced) */
    HT_VO_OUTPUT_1920x1200_60, /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking) */
    HT_VO_OUTPUT_640x480_60, /* VESA 640 x 480 at 60 Hz (non-interlaced) CVT */
    HT_VO_OUTPUT_960H_PAL, /* ITU-R BT.1302 960 x 576 at 50 Hz (interlaced) */
    HT_VO_OUTPUT_960H_NTSC, /* ITU-R BT.1302 960 x 480 at 60 Hz (interlaced) */
    HT_VO_OUTPUT_1920x2160_30, /* 1920x2160_30 */
    HT_VO_OUTPUT_2560x1440_30, /* 2560x1440_30 */
    HT_VO_OUTPUT_2560x1440_60, /* 2560x1440_60 */
    HT_VO_OUTPUT_2560x1600_60, /* 2560x1600_60 */
    HT_VO_OUTPUT_3840x2160_24, /* 3840x2160_24 */
    HT_VO_OUTPUT_3840x2160_25, /* 3840x2160_25 */
    HT_VO_OUTPUT_3840x2160_30, /* 3840x2160_30 */
    HT_VO_OUTPUT_3840x2160_50, /* 3840x2160_50 */
    HT_VO_OUTPUT_3840x2160_60, /* 3840x2160_60 */
    HT_VO_OUTPUT_4096x2160_24, /* 4096x2160_24 */
    HT_VO_OUTPUT_4096x2160_25, /* 4096x2160_25 */
    HT_VO_OUTPUT_4096x2160_30, /* 4096x2160_30 */
    HT_VO_OUTPUT_4096x2160_50, /* 4096x2160_50 */
    HT_VO_OUTPUT_4096x2160_60, /* 4096x2160_60 */
    
    /* HDMI2.1  */
    HT_VO_OUTPUT_7680x4320_24, 
    HT_VO_OUTPUT_7680x4320_25, 
    HT_VO_OUTPUT_7680x4320_30, 
    HT_VO_OUTPUT_7680x4320_50, 
    HT_VO_OUTPUT_7680x4320_60, 

	/* For split mode */
    HT_VO_OUTPUT_3840x1080_60, 

    HT_VO_OUTPUT_USER, /* User timing. */
    HT_VO_OUTPUT_DEFAULT,

    HT_VO_OUTPUT_BUTT
} HT_VO_INTF_SYNC;

typedef enum HT_VI_MODE_E{
    HT_VI_MODE_MIPI_1_LANE,
    HT_VI_MODE_MIPI_2_LANE,
    HT_VI_MODE_MIPI_4_LANE,
    HT_VI_MODE_BT656_1MUX_P,
    HT_VI_MODE_BT656_1MUX_I,
    HT_VI_MODE_BT656_1MUX_DOUBLE_P,
    HT_VI_MODE_BT656_1MUX_DOUBLE_I,
    HT_VI_MODE_BT1120_STANDARD_P,
    HT_VI_MODE_BT1120_STANDARD_I,
    HT_VI_MODE_BT1120_INTERLEAVED_P,
    HT_VI_MODE_BT1120_INTERLEAVED_I,
}HT_VI_MODE;

struct resolution{
    HT_U32 width;
    HT_U32 height;
};

struct rect {
    HT_S32 s32X;
    HT_S32 s32Y;
	struct resolution res;
};

struct ht_vi_info{

	struct resolution res;

	//	海思瑞芯微联合时需要platform输出参数信息
	union{
		HT_VI_MODE vi_mode;
		const char *dev_name;
	};

	//	reserved
	HT_U32 pipe_id;
};

#endif
