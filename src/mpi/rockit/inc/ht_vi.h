/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_vi.h                                            *
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


#ifndef __HT_VI_H
#define __HT_VI_H

#include "ht_type.h"

typedef HT_S32 (*ht_vi_get_frame_cb)(void *user, void *);

HT_S32 ht_vi_dev_init(HT_U32 viDev);
HT_S32 ht_vi_dev_exit(HT_U32 viDev);

HT_S32 ht_vi_chn_init(HT_U32 viDev, HT_U32 viChn, struct ht_vi_info *);
HT_S32 ht_vi_chn_get_frame(HT_U32 viDev, HT_U32 viChn, HT_S32 wait_time, ht_vi_get_frame_cb cb, void *user);

HT_S32 ht_vi_chn_exit(HT_U32 viDev, HT_U32 viChn);

#endif
