/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_tde.h                                           *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-31                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef __HT_TDE_H
#define __HT_TDE_H

#include "ht_type.h"

struct ht_tde_pixel_format{

	struct resolution src_res;
	
	HT_PIXEL_FORMAT src_format;
	HT_PIXEL_FORMAT dst_format;
};

struct ht_tde_resolution{

	struct resolution src;

	struct resolution dst;

	HT_PIXEL_FORMAT src_format;
};


HT_S32 ht_tde_init(void);
HT_S32 ht_tde_pixel_format(HT_U32 vpss_grp, HT_U32 vpss_chn, struct ht_tde_pixel_format *htpf, cb_tde_get_frame cb, void *user);


#endif
