/*
 * MediaFrameSink.h
 *
 *  Created on: 2016年3月16日
 *      Author: terry
 */

#ifndef MEDIAFRAMESINK_H_
#define MEDIAFRAMESINK_H_

#include "MediaFormat.h"

#ifndef AV_MEDIAFRAME_H_
#define	AV_MEDIAFRAME_H_

struct MediaFrame
{

	int type;	///@see MediaType, 用于区分音频/视频
	int fmt;	/// 帧格式

	unsigned char* data[8];	/// 媒体数据, 对于视频, 每个元素指向一个平面;对于音频, 元素指向通道
	int linesize[8];		/// 第几个平面|通道的字节数

	int64_t pts;	/// 时间戳
	int flags;		/// 标记

	/// for video only
	int width;		/// 视频高
	int height;		/// 视频宽

	/// for audio only
	int channels;	/// 音频通道
	int sampleRate;	/// 采样率
	int sampleBits;	/// 采用位数, 取值为8或者16
    int samples;    /// 采样数
};

#endif // AV_MEDIAFRAME_H_


namespace av
{



class MediaFrameSink
{
public:
	virtual ~MediaFrameSink() {}

    /**
     * 接收媒体流格式
     * @param format
     * @return void
     */
    virtual void onMediaFrameFormat(const MediaFormat& format) =0;

    /**
     * 接收解码帧
     * @param frame 不能改写帧内容
     */
    virtual void onMediaFrame(MediaFrame& frame) =0;

    /**
     * 媒体流结束
     * @return void
     */
    virtual void onMediaFrameEnd() =0;

};



} /* namespace av */

#endif /* MEDIAFRAMESINK_H_ */
