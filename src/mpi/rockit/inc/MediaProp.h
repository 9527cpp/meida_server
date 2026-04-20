/*
 * MediaProp.h
 *
 *  Created on: 2016年5月18日
 *      Author: terry
 */

#ifndef INCLUDE_MEDIAPROP_H_
#define INCLUDE_MEDIAPROP_H_


#define	MEDIA_SOURCE_PROP		"source"		/// 媒体源属性
#define	MEDIA_VIDEO_PROP		"video"			/// 视频属性
#define MEDIA_AUDIO_PROP		"audio"			/// 音频属性
#define	MEDIA_VFILTER_PROP		"vfilter"		/// 视频过滤器属性
#define	MEDIA_PLAYER_PROP		"player"		///



/// 最大延时, 默认为 -1
#define	MEDIA_SOURCE_PROP_MAX_DELAY		"source.max_delay"

/// 媒体格式探测字节数, 取值[0,1024000], 取值较小, 可以快速出图
#define	MEDIA_SOURCE_PROP_PROBESIZE		"source.probesize"


/// 取值 tcp 表示使用RTP over TCP
#define MEDIA_SOURCE_PROP_RTSP_TRANSPORT	"source.rtsp_transport"

#define	MEDIA_VFILTER_PROP_BLACK_DETECT		"vfilter.blackdetect"

/// 不启用音视频同步
#define	MEDIA_VIDEO_PROP_NOSYNC				"video.nosync"

/// 视频显示模块
#define	MEDIA_VIDEO_PROP_RENDER				"video.render"

/// 禁止全屏
#define	MEDIA_VIDEO_PROP_ENABLE_FULLSCREEN	"video.fullscreen"

/// 显示时钟
#define	MEDIA_VIDEO_PROP_SHOW_CLOCK	    "video.showclock"


#define	MEDIA_PLAYER_PROP_LIVE_MODE		"player.livemode"

#define MEDIA_PLAYER_PROP_MAX_DURATION  "player.maxduration"

/// 自动录像, 属性值为文件路径
#define MEDIA_PLAYER_PROP_RECORD        "player.record"

/// 自动导出TS文件, 属性值为文件路径
#define MEDIA_PLAYER_PROP_EXPORT        "player.export"




#endif /* INCLUDE_MEDIAPROP_H_ */
