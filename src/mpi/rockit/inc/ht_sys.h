/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_sys.h                                           *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-10                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef __HT_SYS_H
#define __HT_SYS_H


#include "ht_type.h"

HT_S32 ht_sys_init(void);
HT_S32 ht_sys_exit(void);

HT_S32 ht_sys_bind(MOD_ID src_mid, HT_U32 src_dev, HT_U32 src_chn, MOD_ID dst_mid, HT_U32 dst_dev, HT_U32 dst_chn);

#endif
