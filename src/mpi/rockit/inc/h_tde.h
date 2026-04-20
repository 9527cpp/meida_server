/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : h_tde.h                                            *
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


#ifndef __H_TDE_H
#define __H_TDE_H


#include "h_register.h"
#include "ht_tde.h"

#define HT_MPP_HANDLE_TDE	(struct ht_tde *)HT_MPP_HANDLE(HT_ID_TDE)

struct ht_tde_config{
	HT_S32 vpss_grp;
	HT_S32 vpss_chn;

	cb_tde_get_frame user_cb;

	void *user;

	struct ht_tde_pixel_format config_pixel_format;
	
	pthread_mutex_t pmutex;

	pthread_t pthread_id;
	
	struct ht_tde_config *next;
};

struct ht_tde{

	pthread_mutex_t config_mutex;
	
	struct ht_tde_config *config;

	HT_S32 (*tde_init)(struct ht_tde *ptde);
	void (*tde_exit)(struct ht_tde *ptde);

/*
**	将前景位图（pstForeGround） 与背景位图（pstBackGround）的指定区域 (pstForeGroundRect、 pstBackGroundRect） 进行运算， 将运算后的位图拷贝到目标位图
**（pstDst）的指定区域（pstDstRect）中。 其中背景位图（pstBackGround）的指定区域 (pstBackGroundRect）和目标目标位图（pstDst）的指定区域（pstDstRect）必须一致。
**
**	支持指定区域旋转、裁剪、缩放、colorkey和叠加操作。暂时不支持ROP操作。
**
**	Alpha 混合操作
**	有两种方式 一种是将前景位图和背景位图根据配置的混合模型进行Alpha叠加计算，然后输出到背景位图上面。另一种是将背景位图和前景位图进行Alpha叠加后输出到目标位图。
**
**	ColorKey 操作
**	ColorKey技术是对源图像进行预处理，将符合色键过滤条件的像素的alpha分量置零，其中所述色键过滤条件为透明的颜色值，并将预处理后的源图像与目标图像进行alpha混合模式。 
**	RK356x仅支持对前景进行ColorKey的操作。
*/
	HT_S32 (*tde_new_pixel_format)(struct ht_tde *ptde, HT_U32 vpss_grp, HT_U32 vpss_chn, struct ht_tde_pixel_format *htpf, cb_tde_get_frame cb, void *user);

	HT_S32 (*tde_new_resolution)(struct ht_tde *ptde, HT_U32 vpss_grp, HT_U32 vpss_chn, struct ht_tde_resolution *htr, cb_tde_get_frame cb, void *user);
};


HT_VOID ht_tde_config_init(struct ht_tde *ptde);

#endif
