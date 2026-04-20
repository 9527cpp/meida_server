/*
 * MediaType.h
 *
 *  Created on: 2016-1-12
 *      Author: terry
 */

#ifndef MEDIATYPE_H_
#define MEDIATYPE_H_

#include "BasicType.h"

#ifndef MKTAG
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#endif //MKTAG


namespace av
{

enum MediaType
{
    MEDIA_TYPE_NONE  = -1,
    MEDIA_TYPE_VIDEO = 0,
    MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_DATA,
	AVMEDIA_TYPE_SUBTITLE
};


enum StreamState
{
    STATE_STOPPED = 0,
	STATE_PAUSED,
	STATE_PLAYING
};


enum StreamEvent
{
	STREAM_EVENT_END = 0x10

};


enum MediaCodec
{
    MEDIA_CODEC_NONE = 0,

    MEDIA_CODEC_MPEG2 = 2,
    MEDIA_CODEC_MPEG4 = 13,

    MEDIA_CODEC_H264 = 28,
    MEDIA_CODEC_VP8 = 141,

    MEDIA_CODEC_THEORA = 31,
    MEDIA_CODEC_H265 = 174,
    MEDIA_CODEC_HEVC = MEDIA_CODEC_H265,

    MEDIA_CODEC_G711U = 65542,
    MEDIA_CODEC_G711A,

    MEDIA_CODEC_G722 = 69660,
    MEDIA_CODEC_G723 = 86069,
    MEDIA_CODEC_G729 = 86070,
    MEDIA_CODEC_G726 = 69643,
    MEDIA_CODEC_MP2 = 0x15000,

    MEDIA_CODEC_MP3 = 0x15001,
    MEDIA_CODEC_AAC = 0x15002,
    MEDIA_CODEC_AC3 = 0x15003,
    MEDIA_CODEC_VORBIS = 0x15005,
	MEDIA_CODEC_OPUS = 0x1503d,
    MEDIA_CODEC_PS = MKTAG('M', 'P', '2', 'P'), /// PS stream

    MEDIA_CODEC_RAW = 0x10101010

};

enum MediaFlag
{
	MEDIA_FLAG_KEY = 0x01
};


/// sample format

static const float MAX_SCALE = 8.0;
static const float MIN_SCALE = 1.0/8;
static const float STEP_SCALE = 1.0/4;


} /* namespace av */

#endif /* MEDIATYPE_H_ */
