/*
 * MediaEventSink.h
 *
 *  Created on: 2016年3月16日
 *      Author: terry
 */

#ifndef MEDIAEVENTSINK_H_
#define MEDIAEVENTSINK_H_

#include "MediaEvent.h"

namespace av
{

class LitePlayer;

class MediaEventSink
{
public:
	virtual ~MediaEventSink() {}

	/**
	 * 接收到媒体事件
	 * @param type		事件类型
	 * @param value		事件附加值
	 */
	virtual void onMediaEvent(LitePlayer* player, int type, int64_t value) =0;

};



} /* namespace av */

#endif /* MEDIAEVENTSINK_H_ */
