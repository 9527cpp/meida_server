/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_vpss.h                                          *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-9-2                                           *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __HT_VPSS_H
#define __HT_VPSS_H

#include "ht_type.h"

/*
**
**	ht_vpss_config_chn
**	注意：在调用ht_vpss_init前调用，否则无效果。
**	@vpss_grp 组号
**	@vpss_chn 通道号
**	@width	当前通道输出的目标宽
**	@heigth 当前通道输出的目前高
**	@depth_cnt 需要从当前通道getchnframe时，需要将depth_cnt设置为大于0的值，建议8
*/
HT_S32 ht_vpss_config_chn(HT_U32 vpss_grp, HT_U32 vpss_chn, HT_U32 width, HT_U32 height, HT_U32 depth_cnt);


HT_S32 ht_vpss_init(HT_U32 vpss_grp, HT_PIXEL_FORMAT src_pixel_format);
HT_S32 ht_vpss_exit(HT_U32 vpss_grp);

#endif
