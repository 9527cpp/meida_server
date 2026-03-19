#ifndef __STREAM_INTF_H__
#define __STREAM_INTF_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 流数据回调：数据产出时通知监听者 */
typedef void (*stream_data_cb)(void *userdata, const char *data, int len);

#ifdef __cplusplus
}
#endif

#endif /* __STREAM_INTF_H__ */
