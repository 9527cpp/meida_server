/*
 * MediaEvent.h
 *
 *  Created on: 2016年3月16日
 *      Author: terry
 */

#ifndef MEDIAEVENT_H_
#define MEDIAEVENT_H_

#include "MediaType.h"

namespace av
{

enum MediaEventType
{
	MEDIA_EVENT_OPENING = 0x0100,	/// 打开文件中
	MEDIA_EVENT_OPEN_FAILED,		/// 打开失败
	MEDIA_EVENT_OPENED,			/// 打开成功
	MEDIA_EVENT_CLOSED,			/// 文件关闭

	MEDIA_EVENT_END,		/// 播放结束

	MEDIA_EVENT_FORMAT_READY,	/// 已知媒体格式

	MEDIA_EVENT_SEEKING,	/// 拖拽中
	MEDIA_EVENT_SEEKED,		/// 拖拽完成

	MEDIA_EVENT_FULLSCREEN,	/// 全屏变化
	MEDIA_EVENT_VOLUME,		/// 音量变化
	MEDIA_EVENT_MUTE,		/// 静音变化
	MEDIA_EVENT_SCALE,		/// 播放速度变化

	MEDIA_EVENT_PLAYING,		/// 播放中
	MEDIA_EVENT_PAUSED,			/// 暂停
	MEDIA_EVENT_STOPPED,		/// 停止播放

	MEDIA_EVENT_BLACK_START,	/// 黑场开始
	MEDIA_EVENT_BLACK_END,		/// 黑场结束

	MEDIA_EVENT_PACKET,

    MEDIA_EVENT_STREAM_BREAK = 0x0120   /// 流间断
};





} /* namespace av */

#endif /* MEDIAEVENT_H_ */
