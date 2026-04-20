/*
 * MediaSource.h
 *
 *  Created on: 2016-3-17
 *      Author: terry
 */

#ifndef MEDIASOURCE_H_
#define MEDIASOURCE_H_

#include "MediaSample.h"

namespace av
{


class MediaSource
{
public:
	virtual ~MediaSource() {}

	virtual int open(const std::string& url, const std::string& params) =0;

	virtual void close() =0;

	virtual bool isOpen() =0;

	virtual bool getFormat(MediaFormat& fmt) =0;

    virtual int getDuration() =0;

	virtual int play() =0;
	virtual int pause() =0;
	virtual void stop() =0;

	virtual int getState() =0;

	virtual bool seekable() =0;

	virtual int seek(int64_t offset) =0;

	virtual int64_t getTime() =0;

	virtual int setScale(float scale) =0;

	virtual float getScale() =0;

	virtual int read(AVPacket& pkt) =0;

    virtual void interrupt() =0;

    virtual bool isLive() =0;


};



} /* namespace av */

#endif /* MEDIASOURCE_H_ */
