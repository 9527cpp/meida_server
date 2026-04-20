/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_vo.h                                            *
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


#ifndef __HT_VO_H
#define __HT_VO_H

#include "ht_type.h"

HT_S32 ht_vo_dev_init(HT_U32 vo_dev, HT_U32 vo_layer, HT_VO_INTF_SYNC inf_sync,  HT_VO_INTF_TYPE inf_type, HT_VO_LAYER_MODE mode);
HT_S32 ht_vo_dev_exit(HT_U32 vo_dev, HT_U32 vo_layer);
HT_S32 ht_vo_chn_init(HT_U32 vo_chn, HT_U32 vo_layer, struct rect *rec);
HT_S32 ht_vo_chn_exit(HT_U32 vo_chn, HT_U32 vo_layer);


#endif
