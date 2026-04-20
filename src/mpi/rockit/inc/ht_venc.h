/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_venc.h                                          *
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
#ifndef __HT_VENC_H
#define __HT_VENC_H

#include "ht_type.h"

HT_S32 ht_venc_config_chn(HT_U32 chn);

HT_S32 ht_venc_init(HT_U32 chn, HT_U32 u32SrcWidth, HT_U32 u32SrcHeight, HT_CODEC_ID codec);
HT_S32 ht_venc_get_frame(HT_U32 chn, cb_venc_get_frame cb_user, void *user);
HT_S32 ht_venc_chn_send_frame(HT_U32 chn, void *frame, HT_S32 msec);


HT_S32 ht_venc_exit(HT_U32 chn);


#endif

