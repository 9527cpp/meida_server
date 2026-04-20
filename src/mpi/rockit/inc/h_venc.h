/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_venc.h                                           *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-17                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef __H_VENC_H
#define __H_VENC_H

#include "h_register.h"


#define HT_MPP_HANDLE_VENC	(struct ht_venc *)HT_MPP_HANDLE(HT_ID_VENC)

struct ht_venc{

	HT_S32 (*venc_init)(struct ht_venc *, HT_U32 chn, HT_U32 u32SrcWidth, HT_U32 u32SrcHeight, HT_CODEC_ID codec);
	HT_S32 (*venc_exit)(struct ht_venc *, HT_U32 chn);

	HT_S32 (*venc_get_frame)(struct ht_venc *, HT_U32 chn, cb_venc_get_frame, void *user);
	HT_S32 (*venc_send_frame)(struct ht_venc *, HT_U32 chn, void *frame, HT_S32 s32MilliSec);
};

#endif

