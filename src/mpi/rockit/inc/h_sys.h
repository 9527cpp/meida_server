/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_sys.h                                            *
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


#ifndef __H_SYS_H
#define __H_SYS_H

#include "h_register.h"

#define HT_MPP_HANDLE_SYS	(struct ht_sys *)HT_MPP_HANDLE(HT_ID_SYS)

struct ht_sys{
	HT_S32 (*ht_sys_init)();
	HT_S32 (*ht_sys_exit)();

	HT_S32 (*ht_sys_bind)(MOD_ID src_mid, HT_U32 src_dev, HT_U32 src_chn, MOD_ID dst_mid, HT_U32 dst_dev, HT_U32 dst_chn);
	HT_S32 (*ht_sys_unbind)(MOD_ID src_mid, HT_U32 src_dev, HT_U32 src_chn, MOD_ID dst_mid, HT_U32 dst_dev, HT_U32 dst_chn);
	
};

#endif
