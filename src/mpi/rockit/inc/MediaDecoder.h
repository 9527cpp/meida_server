/*
 * MediaDecoder.h
 *
 *  Created on: 2016年3月17日
 *      Author: terry
 */

#ifndef MEDIADECODER_H_
#define MEDIADECODER_H_

#include "MediaSample.h"


namespace av
{

class MediaDecoderSink
{
public:
	virtual ~MediaDecoderSink() {}

	virtual void onDecodeFrame(AVFramePtr frame) =0;

};


class MediaDecoder
{
public:
    virtual ~MediaDecoder() {}

    virtual int open(const MediaFormat& fmt) =0;

    virtual void close() =0;

    virtual bool isOpen() =0;

    virtual int input(AVPacketPtr pkt) =0;

    virtual void setSink(MediaDecoderSink* sink) =0;

    virtual void flush() =0;

    virtual bool getOutFormat(RawMediaFormat& fmt) =0;

};



} /* namespace av */

#endif /* MEDIADECODER_H_ */
