/*
 * LibMediaReader.h
 *
 *  Created on: 2016年8月12日
 *      Author: zhengboyuan
 */

#ifndef LIBMEDIAREADER_H_
#define LIBMEDIAREADER_H_


////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

    #ifndef NOMINMAX
    #define NOMINMAX
    #endif //NOMINMAX

	#include <Windows.h>
#else

#endif //WIN32


////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    typedef signed char     int8_t;
    typedef unsigned char   uint8_t;
    typedef short           int16_t;
    typedef unsigned short  uint16_t;
    typedef int             int32_t;
    typedef unsigned        uint32_t;
    typedef long long       int64_t;
    typedef unsigned long long   uint64_t;
#else
    #include <stdint.h>
    typedef void*   HANDLE;
#endif //_MSC_VER


///////////////////////////////////////////////////////////////////
#ifdef WIN32
    #ifndef DLLEXPORT
    #define DLLEXPORT __declspec(dllexport)
    #endif //DLLEXPORT
#else
    #define DLLEXPORT __attribute__ ((visibility ("default")))
#endif //WIN32

///////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

/////////////////////////////////////////////////////////////////////////////
#ifndef MKBETAG
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#endif //MKBETAG


/////////////////////////////////////////////////////////////////////////////
#ifndef MKBETAG
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#endif //MKBETAG

#ifndef CASTER_TYPE
#define	CASTER_TYPE

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

/// 编码.
enum MCodec
{
	MCODEC_NONE = 0,

	MCODEC_H264 = 28,
	MCODEC_HEVC = 174, /// H.265
	MCODEC_H265 = MCODEC_HEVC,

	MCODEC_G711U = 65542,
	MCODEC_G711A,

	MCODEC_MP3 = 0x15001,
	MCODEC_AAC = 0x15002,
	MCODEC_AC3 = 0x15003,
	MCODEC_VORBIS = 0x15005,
	MCODEC_OPUS = 0x1503d,
	MCODEC_RAW = 0x10101010

};

enum MType
{
	MTYPE_NONE = -1,
	MTYPE_VIDEO = 0,
	MTYPE_AUDIO,
	MTYPE_DATA,
};


/// 媒体格式 
struct MFormat
{
	int codec;		/// 视频编码  @see MCodec
	int width;		/// 视频高 
	int height;		/// 视频宽 
	int framerate;		/// 帧率 
	int profile;
	int clockRate;  /// 时钟频率 

	int audioCodec;	/// 音频编码  @see MCodec
	int channels;	/// 通道数 
	int sampleRate;	/// 采样率 
	int audioProfile;	/// 档次 
	int audioRate;      /// 音频时钟频率 

	int vPropSize;		/// 视频解码参数, 对于H.264是sps+pps, 对于H.265是vps+sps+pps
	unsigned char* vProp;

	int configSize;		/// 音频解码参数, 如果是AAC编码, 则必须设置为AAC的config参数 
	unsigned char* config;

	int bitrate;    ///码率. 
	int audioBitrate;
};


/// 媒体包.
struct MPacket
{
	int type;       ///
	uint8_t* data;	/// 数据指针.
	int size;		/// 数据长度.
	int64_t pts;	/// 时间戳.
	int duration;	/// 时长.
	int flags;		/// 标识.
};


enum CasterConst
{
	MAX_CASTER = 64,
	INVALID_CASTER = -1
};


typedef int		caster_t;


enum CasterEventType
{
	CASTER_SESSION_REQUEST = 1,
	CASTER_SESSION_CREATE,
	CASTER_SESSION_DESTROY,
};

struct CasterEvent
{
	int  type;
	caster_t  handle;	/// 通道句柄.
	char name[256];		/// 通道名称.
};


/**
* 事件回调函数.
* @param event		事件.
* @param context	回调环境.
*/
typedef void(*CasterEventCallback)(struct CasterEvent* event, void* context);


#endif //CASTER_TYPE


enum MReaderConst
{
	MAX_READER = 128,
	INVALID_READER = -1
};

typedef int		mreader_t;

/**
 * 事件回调
 * @param reader
 * @param event     事件类型
 * @param value     事件关联值. 打开失败时, event = MEDIA_EVENT_OPEN_FAILED, value 表示错误码. ETIMEDOUT 表示超时, ECONNREFUSED 表示用户/密码错误.
 * @param context
 */
typedef void (*MReaderEventCallback)(mreader_t reader, int event, int64_t value, void* context);


#define MREADER_MAX_SCALE 	8.0f
#define MREADER_MIN_SCALE 	1.0f/8
#define MREADER_STEP_SCALE 	1.0f/4


enum MReaderStreamState
{
    MREADER_STATE_STOPPED = 0,
	MREADER_STATE_PAUSED,
	MREADER_STATE_PLAYING
};


/////////////////////////////////////////////////////////////////////////////

/**
 * 初始化
 * @return
 */
DLLEXPORT int mreader_init();

/**
 *
 */
DLLEXPORT void mreader_quit();

/**
 * 打开媒体源, url 可以是网络源或者本地文件路径.
 * urtp:// 是RTP协议流, 格式为: rtp://ip?port=x&payload=x&codec=x&sprop=x&aport=y&apayload=y&acodec=y&aclock=y. 比如:
 * urtp://127.0.0.1/?port=10000&payload=96&codec=H264&aport=10002&apayload=0&acodec=AAC&aclock=16000&achl=1&protocol=udp;
 * protocol=rtcp|rtp|udp|tcp, protocol 取值为rtp或者rtcp时, 启用RTCP.
 * codec 可以是 H264/H265/HEVC
 * @param handle 	返回的句柄
 * @param url		媒体源URL
 * @param params	可选参数. ';' 分割的赋值字符串.
 * 	probesize=x 表示探测格式需要读取字节数;
 *  stimeout=5000000 表示连接超时, 单位为1/1000000;
 *  rtsp_transport=tcp 表示rtsp协议时使用tcp传输媒体;
 * @return 0 表示成功, 其他值表示错误码
 */
DLLEXPORT int mreader_open(mreader_t* handle, const char* url, const char* params);

/**
 * 关闭媒体源
 * @param handle
 * @return
 */
DLLEXPORT int mreader_close(mreader_t handle);

/**
 * 媒体源是否打开
 * @param handle
 * @return > 0 表示已经打开
 */
DLLEXPORT int mreader_isOpen(mreader_t handle);

/**
 * 获取媒体格式
 * @param handle
 * @param fmt
 * @return
 */
DLLEXPORT int mreader_getFormat(mreader_t handle, struct MFormat* fmt);

/**
 * 获取媒体时长
 * @param handle	媒体源句柄
 * @return 媒体时长, 单位为毫秒
 */
DLLEXPORT int mreader_getDuration(mreader_t handle);

/**
 * 是否为实时媒体源
 * @param handle
 * @return > 0 表示为实时媒体源
 */
DLLEXPORT int mreader_isLive(mreader_t handle);

/**
 * 是否可以定位
 * @param handle
 * @return > 0 表示可定位
 */
DLLEXPORT int mreader_seekable(mreader_t handle);

/**
 * 播放媒体源
 * @param handle
 * @return
 */
DLLEXPORT int mreader_play(mreader_t handle);

/**
 * 暂停媒体源
 * @param handle
 * @return
 */
DLLEXPORT int mreader_pause(mreader_t handle);

/**
 * 停止媒体源 
 * @param handle
 * @return
 */
DLLEXPORT int mreader_stop(mreader_t handle);

/**
 * 获取媒体源状态 
 * @param handle
 * @return
 */
DLLEXPORT int mreader_getState(mreader_t handle);

/**
 * 读取媒体包, pkt 中的数据缓冲区需要自己分配, 长度应  > 512K.
 * @param handle
 * @param pkt
 * @return 0 表示成功, EAGAIN 表示无数据, 需要再次调用. EOVERFLOW 表示缓冲区长度不足.
 */
DLLEXPORT int mreader_read(mreader_t handle, struct MPacket* pkt);

/**
 * 中断读取操作 
 * @param handle
 * @return
 */
DLLEXPORT int mreader_interrupt(mreader_t handle);


/**
 * 定位到指定偏移, 单位为毫秒
 * @param handle
 * @param offset
 * @return
 */
DLLEXPORT int mreader_seek(mreader_t handle, int64_t offset);

/**
 * 获取当前时间戳, 单位为微秒
 * @param handle
 * @return 时间戳
 */
DLLEXPORT int64_t mreader_getTime(mreader_t handle);

/**
 * 开始记录为文件 
 * @param handle
 * @param filename	录像文件 
 * @return
 */
DLLEXPORT int mreader_startRecord(mreader_t handle, const char* filename);

/**
 * 停止记录
 * @param handle
 * @return
 */
DLLEXPORT int mreader_stopRecord(mreader_t handle);

/**
 * 是否正在记录
 * @param handle
 * @return > 0 表示正在记录 
 */
DLLEXPORT int mreader_isRecording(mreader_t handle);


/**
 * 设置事件回调 
 * @param handle
 * @param cb
 * @param context
 * @return
 */
DLLEXPORT int mreader_setEventCallback(mreader_t handle, MReaderEventCallback cb, void* context);

/**
 * 启用/禁止日志 
 * @param enabled >0 表示启用 
*/
DLLEXPORT void mreader_enableLog(int enabled);

/**
 * 等待媒体包就绪.
 * mreader_read 返回 EAGAIN 时, 可以用它做延时等待
 * @param handle
 * @param ms	毫秒
 * @return
 */
DLLEXPORT int mreader_wait(mreader_t handle, int ms);

/**
 * 设置播放速度，正常播放速度为 1.0. 仅对于文件,VOD流有效
 * @param handle 句柄
 * @param scale 播放速度，取值范围: [1/8, 8]
 * @return 0 表示成功
 */
DLLEXPORT int mreader_setScale(mreader_t handle, float scale);

/**
 * 获取播放速度
 * @param handle
 * @return 播放速度
 */
DLLEXPORT float mreader_getScale(mreader_t handle);

/**
 * 减速播放, 每次减速 MREADER_STEP_SCALE. 仅对于文件,VOD流有效
 * @param handle
 * @return 返回 0表示成功
 */
DLLEXPORT int mreader_slow(mreader_t handle);

/**
 * 加速播放, 每次加速 MREADER_STEP_SCALE . 仅对于文件,VOD流有效
 * @param handle
 * @return 返回0表示成功
 */
DLLEXPORT int mreader_fast(mreader_t handle);


/**
 * 单帧播放.
 * 进入暂停播放状态, 调用一次, 仅解码播放一帧视频
 * @param handle
 * @return 0 表示成功
 */
DLLEXPORT int mreader_step(mreader_t handle);

/**
 * 播放下一关键帧.
 * 进入暂停播放状态, 调用一次, 仅解码播放一个关键帧
 * @param handle
 * @return
 */
DLLEXPORT int mreader_nextKeyFrame(mreader_t handle);

/**
 * 设置线程堆栈大小,
 * @param bytes	字节数
 * @return
 */
DLLEXPORT void mreader_setStackSize(int bytes);


/**
* 回退播放
* @param handle 
* @param offset 时间戳, < 0 表示当前位置. 单位为 毫秒.
* @return
*/
DLLEXPORT int mreader_backward(mreader_t handle, int64_t offset);

/**
* 向前播放
* @param handle
* @return
*/
DLLEXPORT int mreader_forward(mreader_t handle);


/**
* 设置流中断超时时间. (没有收到媒体包的超时时间). 单位为毫秒.
* 流中断超时会发送通知, 通知值为: MEDIA_EVENT_STREAM_BREAK = 0x0120
* @param ms 超时, 毫秒.  <= 0 表示不通知. 默认为0.
* @return
*/
DLLEXPORT void mreader_setStreamBreakTimeout(int ms);

/**
 * 设置缓冲队列内元素最大个数, 默认为 150
 * @param count
 * @return
*/
DLLEXPORT void mreader_setMaxQueueSize(size_t count);

/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif


#endif /* LIBMEDIAREADER_H_ */
