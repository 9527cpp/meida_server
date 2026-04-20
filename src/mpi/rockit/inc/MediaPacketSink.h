/*
 * MediaPacketSink.h
 *
 *  Created on: 2016年3月16日
 *      Author: terry
 */

#ifndef MEDIAPACKETSINK_H_
#define MEDIAPACKETSINK_H_

#include "MediaFormat.h"



#ifndef AV_MEDIAPACKET_H_
#define AV_MEDIAPACKET_H_

struct MediaPacket
{
    int type;

    uint8_t*    data;
    size_t      size;

    int     flags;

    int64_t ts;
    int32_t duration;

};

#endif //AV_MEDIAPACKET_H_


namespace av
{


class MediaPacketSink
{
public:
	virtual ~MediaPacketSink() {}

    /**
     * 接收媒体流格式
     * @param format
     * @return void
     */
    virtual void onMediaFormat(const MediaFormat& format) =0;

    /**
     * 接收解码帧
     * @param packet 不能改写帧内容
     * @return
     */
    virtual void onMediaPacket(MediaPacket& packet) =0;

    /**
     * 媒体流结束
     * @return void
     */
    virtual void onMediaEnd() =0;

};



} /* namespace av */

#endif /* MEDIAPACKETSINK_H_ */
