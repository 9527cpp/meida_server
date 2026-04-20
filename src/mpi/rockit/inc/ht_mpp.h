/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2031 HT  Corporation All rights reserved *         *
*        *************************************************         *
*                                                                  *
* FileName    : ht_mpp.h                                           *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-10-8                                          *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef __HT_MPP_HPP
#define __HT_MPP_HPP

#ifdef __cplusplus
	extern "C"{
#endif

#include "ht_sys.h"
#include "ht_vi.h"
#include "ht_vo.h"
#include "ht_vdec.h"
#include "ht_venc.h"
#include "ht_tde.h"
#include "ht_vpss.h"
#include "ht_avs.h"

#ifdef __cplusplus
}
#endif

class HtMppVi
{

public:
	static HtMppVi *create(int a);
};

class HtMpp
{
public:
	static HtMpp *CreateInstance(void);

	//	vifunctions ...
	HT_S32 ViDevInit(HT_U32 DevID, HtMppVi* v);
	

	
	virtual ~HtMpp();
	
private:
	struct resolution res;
	HtMpp();
	bool Construct();
};


#endif
