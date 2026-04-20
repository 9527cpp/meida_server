/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_register.h                                       *
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

#ifndef __H_REGISTER_H
#define __H_REGISTER_H

#include "h_comm.h"
#include <pthread.h>

#define HT_MPP_HANDLE(x)	ht_registed_ops(x)


struct ht_handle{
	HT_HANDLE handle_cb;

	HT_HANDLE *config;
	
	const char *suffix;
};

extern struct ht_handle g_handle[HT_ID_BUTT];

inline HT_HANDLE __inline ht_registed_ops(MOD_ID id)
{
	if (id >0 && id < HT_ID_BUTT)
		return g_handle[id].handle_cb;

	return NULL;
}

HT_S32 ht_register_ops(MOD_ID id, HT_HANDLE handler);

#endif
