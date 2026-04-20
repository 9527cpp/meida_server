/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2031 HT  Corporation All rights reserved *         *
*        *************************************************         *
*                                                                  *
* FileName    : h_avs.h                                            *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-11-5                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __H_AVS_H
#define __H_AVS_H

#include "h_register.h"


#define HT_MPP_HANDLE_AVS	(struct ht_avs *)HT_MPP_HANDLE(HT_ID_AVS)

struct ht_avs{

	HT_S32 (*avs_init)(struct ht_avs *pavs);

};

#endif
