/*
 * RtpCaster.h
 *
 *  Created on: 2016年4月23日
 *      Author: terry
 */

#ifndef RTPCASTER_H_
#define RTPCASTER_H_


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


/// 协议类型
enum RtpProtocol
{
	kProtocolUdp = 0,
	kProtocolTcp
};


/**
 * 初始化
 * @return 0 表示成功
 */
DLLEXPORT int rtpcaster_init();

/**
 * 反初始化
 * @return
 */
DLLEXPORT int rtpcaster_quit();

/**
 * 设置端口范围
 * @param minPort
 * @param maxPort
 * @return
 */
DLLEXPORT int rtpcaster_set_port_range(int minPort, int maxPort);


/**
 * 打开发送通道
 * @param codec	编码
 * @param clockRate	时钟
 * @param outPort	本地发送端口
 * @param ssrc		同步源
 * @return	NULL 表示失败
 */
DLLEXPORT int rtpcaster_create_channel(HANDLE* handle, int codec, int clockRate, int outPort, int ssrc);

/**
 * 关闭通道
 * @param handle 通道句柄
 */
DLLEXPORT void rtpcaster_destroy_channel(HANDLE handle);


/**
 * 添加发送目标
 * @param handle 通道句柄
 * @param protocol	协议类型
 * @param ip		地址
 * @param port		端口
 * @return	0 表示成功
 */
DLLEXPORT int rtpcaster_add_target(HANDLE handle, int protocol, const char* ip, int port);

/**
 * 删除发送目标
 * @param handle	通道
 * @param protocol	协议
 * @param ip		地址
 * @param port	端口
 * @return 0 表示成功
 */
DLLEXPORT int rtpcaster_remove_target(HANDLE handle, int protocol, const char* ip, int port);


/**
 * 删除所有的发送目标
 * @param handle
 * @param protocol
 * @return
 */
DLLEXPORT int rtpcaster_remove_all_target(HANDLE handle, int protocol);


/**
 * 写媒体数据
 * @param handle
 * @param data		数据指针
 * @param size		数据长度
 * @param pts		时间戳
 * @param flags		标识
 * @return
 */
DLLEXPORT int rtpcaster_write_data(HANDLE handle, uint8_t* data, int size, int64_t pts, int flags);




/**
 * RTP接收端回调
 * @param handle	RTP句柄
 * @param data		媒体包
 * @param size		长度
 * @param pts		时间戳
 * @param flags		标识
 * @param context	环境指针
 */
typedef void (*RtpSinkCallback)(HANDLE handle, uint8_t* data, int size, int64_t pts, int flags, void* context);

/**
 * 打开RTP接收端
 * @param handle	RTP句柄
 * @param protocol	协议 @see RtpProtocol
 * @param port		端口
 * @return 0 表示成功
 */
DLLEXPORT int rtpsink_open(HANDLE* handle, int protocol, int port);

/**
 * 设置接收回调
 * @param handle
 * @param cb		回调函数
 * @param context	回调环境
 */
DLLEXPORT void rtpsink_set_callback(HANDLE handle, RtpSinkCallback cb, void* context);

/**
 * 设置媒体格式
 * @param handle
 * @param codec		编码格式
 * @param clockRate	时钟频率, 视频为90000, 音频为采样率
 * @return
 */
DLLEXPORT int rtpsink_set_format(HANDLE handle, int codec, int clockRate);

/**
 * 关闭RTP
 * @param handle
 */
DLLEXPORT void rtpsink_close(HANDLE handle);

/**
 * 设置缓存队列长度, 单位为RTP包个数.
 * 该队列用户处理RTP包乱序
 * @param handle
 * @param size
 */
DLLEXPORT void rtpsink_set_cache(HANDLE handle, int size);

/**
 * 获取本地端口
 * @param handle
 * @return
 */
DLLEXPORT int rtpsink_get_port(HANDLE handle);


/**
 * 设置媒体格式
 * @param handle
 * @param url
 * @return
 */
//DLLEXPORT int rtpsink_set_format_url(HANDLE handle, const char* url);

/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* RTPCASTER_H_ */
