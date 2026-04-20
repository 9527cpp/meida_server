/*
 * Ffmpeg.h
 *
 *  Created on: 2016年3月17日
 *      Author: terry
 */

#ifndef FFMPEG_H_
#define FFMPEG_H_



#ifdef _MSC_VER
    #pragma warning(disable: 4244)
#endif //WIN32


#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif


extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>

}


#endif /* FFMPEG_H_ */
