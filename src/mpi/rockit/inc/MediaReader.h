/*
 * MediaReader.h
 *
 *  Created on: 2016-8-12
 *      Author: zhengboyuan
 */

#ifndef MEDIAREADER_H_
#define MEDIAREADER_H_

#include "MediaSource.h"


namespace av
{

typedef void (*ReaderEventCallback)(int handle, int event, int64_t value, void* context);


class MediaReader
{
public:
	virtual ~MediaReader() {}

	virtual int open(const std::string& url, const std::string& params) =0;

	virtual void close() =0;

	virtual bool isOpen() =0;

	virtual int getFormat(MediaFormat& fmt) =0;

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

	virtual int read(AVPacketPtr& pkt) =0;

	virtual void interrupt() =0;

	virtual bool isLive() =0;


	virtual int startRecord(const std::string& filename) =0;

	virtual void stopRecord() =0;

	virtual bool isRecording() =0;

	virtual void setEventCallback(ReaderEventCallback cb, void* context) =0;

	virtual void setHandle(int handle) =0;

	virtual bool waitForReadable(int ms) = 0;

	virtual int step() =0;

	virtual int nextKeyFrame() =0;

	virtual int backward(int64_t offset) =0;

	virtual int forward() = 0;

};



}

#endif /* MEDIAREADER_H_ */
