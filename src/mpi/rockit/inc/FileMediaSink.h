/*
 * FileMediaSink.h
 *
 *  Created on: 2016年5月18日
 *      Author: terry
 */

#ifndef CORE_FILEMEDIASINK_H_
#define CORE_FILEMEDIASINK_H_

#include "MediaFormat.h"
#include "MediaSample.h"


namespace av
{

class FileMediaSink
{
public:
	virtual ~FileMediaSink() {}

    virtual bool open(const char* filename) =0;

    virtual void close() =0;

    virtual bool isOpen() =0;


    virtual void onMediaFormat(const MediaFormat& fmt) =0;

    virtual void onMediaPacket(AVPacketPtr& pkt) =0;

    virtual void onMediaEvent(int event) =0;


    virtual int setFile(const char* filename) =0;

    virtual const char* getFile() =0;

};

typedef std::shared_ptr< FileMediaSink >	FileMediaSinkPtr;


}

#endif /* CORE_FILEMEDIASINK_H_ */
