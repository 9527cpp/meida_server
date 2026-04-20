/*
 * AudioDecoder.h
 *
 *  Created on: 2016年3月17日
 *      Author: terry
 */

#ifndef AUDIODECODER_H_
#define AUDIODECODER_H_

#include "MediaDecoder.h"

namespace av
{

class AudioDecoder: public MediaDecoder
{
public:
	virtual ~AudioDecoder() {}

};


} /* namespace av */

#endif /* AUDIODECODER_H_ */
