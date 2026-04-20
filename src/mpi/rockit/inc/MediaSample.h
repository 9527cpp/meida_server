/*
 * MediaSample.h
 *
 *  Created on: 2016年3月17日
 *      Author: terry
 */

#ifndef MediaSample_H_
#define MediaSample_H_


#include "MediaFormat.h"
#include <string>
#include "Ffmpeg.h"
#include "SharedPtr.h"



namespace av
{


typedef std::shared_ptr< AVFrame >		AVFramePtr;

typedef std::shared_ptr< AVPacket >		AVPacketPtr;



}


#endif /* MediaSample_H_ */
