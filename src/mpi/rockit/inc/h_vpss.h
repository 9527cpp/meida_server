/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_vpss.h                                           *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-24                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __H_VPSS_H
#define __H_VPSS_H

#include "h_register.h"

#define HT_MPP_HANDLE_VPSS	(struct ht_vpss *)HT_MPP_HANDLE(HT_ID_VPSS)

struct ht_vpss_config {
	HT_U32 vpss_grp;
	HT_U32 vpss_chn;

	struct resolution res;

	//	当需要从vpss某个chn上getstream时，需要设置depth的深度（大于0）
	HT_U32 u32Depth;

	pthread_t chn_frame_t;

	//	当调用vpss_init时传入的格式参数默认应用到所有chn
	HT_PIXEL_FORMAT src_pixel_format;
	
	struct ht_vpss_config *next;
};

typedef HT_S32 (*ht_vpss_config_cb)(struct ht_vpss_config *, void *user);

struct ht_vpss {
	
	pthread_mutex_t config_mutex;

	struct ht_vpss_config *config;

	HT_S32 (*vpss_init)(struct ht_vpss *pvpss, HT_U32 vpss_grp, HT_PIXEL_FORMAT fmt);

	HT_S32 (*vpss_chn_start_get_frame)(struct ht_vpss *pvpss, HT_U32 vpss_grp, HT_U32 vpss_chn);
	HT_S32 (*vpss_chn_stop_get_frame)(struct ht_vpss *pvpss, HT_U32 vpss_grp, HT_U32 vpss_chn);
	
	HT_S32 (*vpss_exit)(struct ht_vpss *, HT_U32 vpss_chn);
};

HT_VOID ht_vpss_config_init(struct ht_vpss *pvpss);

#endif
