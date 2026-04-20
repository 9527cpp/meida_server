/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : vi.h                                               *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-19                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef __RV1126_RV1109_VI_H
#define __RV1126_RV1109_VI_H

#include "h_vi.h"

struct rv1126_rv1109_vi{

	//	压缩模式
	COMPRESS_MODE_E enCompressMode;

	//	输出通道总的缓存块数。
	RK_U32 isp_buf_count;

	//	获取图像的V4L2内存类型。
	VI_V4L2_MEMORY_TYPE	enMemoryType;

	//	获取图像的V4L2录制类型。
	VI_V4L2_CAPTURE_TYPE  enCaptureType;

	//	目标图像像素格式。
	PIXEL_FORMAT_E enPixelFormat;

	//	用户获取图像的队列深度
	RK_U32 u32Depth;
	
	struct ht_vi m_vi;
	
};



#endif

