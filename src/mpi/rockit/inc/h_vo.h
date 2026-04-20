/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_vo.h                                             *
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


#ifndef __H_VO_H
#define __H_VO_H

#include "h_register.h"


#define HT_MPP_HANDLE_VO	(struct ht_vo *)HT_MPP_HANDLE(HT_ID_VO)

struct ht_vo{
	HT_S32 (*vo_dev_init)(struct ht_vo *, HT_U32 vo_dev, HT_S32 vo_layer, HT_U32 inf_sync, HT_U32 inf_type, HT_VO_LAYER_MODE layer_mode);
	HT_S32 (*vo_dev_exit)(struct ht_vo *, HT_U32 vo_dev, HT_U32 vo_layer);
	
	HT_S32 (*vo_chn_init)(struct ht_vo *, HT_U32 vo_chn, HT_U32 vo_layer, struct rect *rec);
	HT_S32 (*vo_chn_exit)(struct ht_vo *, HT_U32 vo_chn, HT_U32 vo_layer);

	HT_S32 (*vo_get_layer_size)();

	HT_S32 (*vo_wbc_init)();
	HT_S32 (*vo_wbc_exit)();
};

#endif
