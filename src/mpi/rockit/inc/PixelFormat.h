/*
 * PixelFormat.h
 *
 *  Created on: 2016年1月16日
 *      Author: terry
 */

#ifndef PIXELFORMAT_H_
#define PIXELFORMAT_H_


#include "BasicType.h"

namespace av
{

class DLLEXPORT PixelFormat
{
public:
	enum Format
	{
		kNONE = -1,

	    kI420,  /// yuv 4:2:0 planar, AV_PIX_FMT_YUV420P
		kYUY2,  /// packed YUV 4:2:2, AV_PIX_FMT_YUYV422
		kRGB,   /// RGB24, AV_PIX_FMT_RGB24
		kBGR,	/// AV_PIX_FMT_BGR24
		kI422,	/// AV_PIX_FMT_YUV422P
		kI444,  /// AV_PIX_FMT_YUV444P

	    kYV12 = 12,  /// yvu 4:2:0 planar, AV_PIX_FMT_YUVJ420P

		kUYVY = 17,		/// packed YUV 4:2:2, AV_PIX_FMT_UYVY422

	    kNV12 = 25,  /// yuv 4:2:0, with one y plane and one packed u+v, AV_PIX_FMT_NV12

	    kRGB32= 28,		/// AV_PIX_FMT_RGBA
	    kBGRA = 30,		/// AV_PIX_FMT_BGRA


	    kMAX /* end of list */
	};

public:
	PixelFormat();
	virtual ~PixelFormat();

    static int getPictureSize(int fmt, int width, int height);

    static int getPicturePlaneSize(int fmt, int width, int height, int plane);

    static const char* getName(int fmt);

    static int parseName(const char* name);

};



} /* namespace av */

#endif /* PIXELFORMAT_H_ */
