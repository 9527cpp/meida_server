/*
 * MediaObject.h
 *
 *  Created on: 2016年3月18日
 *      Author: terry
 */

#ifndef MEDIAOBJECT_H_
#define MEDIAOBJECT_H_

#include "MediaSource.h"
#include "MediaDecoder.h"
#include "VideoDecoder.h"
#include "AudioDecoder.h"
#include "SharedPtr.h"


namespace av
{

typedef std::shared_ptr< MediaSource >		MediaSourcePtr;

typedef std::shared_ptr< MediaDecoder >		MediaDecoderPtr;

typedef std::shared_ptr< VideoDecoder >		VideoDecoderPtr;

typedef std::shared_ptr< AudioDecoder >		AudioDecoderPtr;


}


#endif /* MEDIAOBJECT_H_ */
