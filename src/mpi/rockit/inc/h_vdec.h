/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_vdec.h                                           *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-8                                           *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __H_VDEC_H
#define __H_VDEC_H

#include "h_register.h"


#define HT_MPP_HANDLE_VDEC	(struct ht_vdec *)HT_MPP_HANDLE(HT_ID_VDEC)

struct ht_vdec{

	HT_S32 (*vdec_init)(struct ht_vdec *pvdec, HT_U32 chn, HT_U32 u32SrcWidth, HT_U32 u32SrcHeight, HT_CODEC_ID codec);
	HT_S32 (*vdec_sendstream)(struct ht_vdec *pvdec, HT_U32 chn, HT_U8 *addr, HT_S32 size, HT_U64 pts);
	HT_S32 (*vdec_exit)(struct ht_vdec *pvdec, HT_S32 chn);

	//	configs ...
	HT_VOID (*vdec_config_sendstream_timeout)(struct ht_vdec *pvdec, HT_S32 timeout);
};

#endif
