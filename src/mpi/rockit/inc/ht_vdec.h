/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_vdec.h                                          *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                           *
*                                                                  *
* Date        : 2022-8-8                                           *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __HT_VDEC_H
#define __HT_VDEC_H

#include "ht_type.h"

HT_S32 ht_vdec_init(HT_U32 chn, HT_U32 u32SrcWidth, HT_U32 u32SrcHeight, HT_CODEC_ID codec);
HT_S32 ht_vdec_exit(HT_U32 chn);
HT_S32 ht_vdec_sendstream(HT_U32 chn, HT_U8 *addr, HT_S32 size, HT_U64 pts);

#endif
