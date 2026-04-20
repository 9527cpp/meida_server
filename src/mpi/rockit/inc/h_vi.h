/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_vi.h                                             *
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


#ifndef __H_VI_H
#define __H_VI_H

#include "h_register.h"
#include "ht_vi.h"

#define HT_MPP_HANDLE_VI	(struct ht_vi *)HT_MPP_HANDLE(HT_ID_VI)

struct ht_vi_config {

	/*
	**	dev_id chn_id为-1时
	**	配置应用于所有通道
	*/
	HT_S32 dev_id;
	HT_S32 chn_id;

	/*

	*/
	HT_S32 u32Depth;

	struct ht_vi_config *next;
};

struct ht_vi{

	MOD_ID m_id;

	struct ht_vi_config pconfig;

	HT_S32 (*vi_dev_init)(struct ht_vi *pvi, HT_U32 viDev);
	HT_S32 (*vi_dev_exit)(struct ht_vi *pvi, HT_U32 viDev);
	
	HT_S32 (*vi_chn_init)(struct ht_vi *pvi, HT_U32 viDev, HT_U32 viChn, struct ht_vi_info *);
	HT_S32 (*vi_chn_get_frame)(struct ht_vi *pvi, HT_U32 viPipe, HT_U32 viChn, HT_S32 wait_time, ht_vi_get_frame_cb cb, void *user);
	HT_S32 (*vi_chn_exit)(struct ht_vi *pvi, HT_U32 viDev, HT_U32 viChn);
};

#endif
