#ifndef __MEDIA_EVENTS_HPP__
#define __MEDIA_EVENTS_HPP__

#include "stream/stream_listener.hpp"

enum class media_event_type {
    start_video, /* 启动视频流 */
    assign_video, /* 关联视频通道 */
    stop_video, /* 停止视频流 */
    unassign_video, /* 取消关联视频通道 */
    start_audio, /* 启动音频流 */
    assign_audio, /* 关联音频通道 */
    stop_audio, /* 停止音频流 */
    unassign_audio, /* 取消关联音频通道 */
};

struct media_event {
    media_event_type type;
    int chn;
    stream_listener *listener; /* video 时使用；audio 可为 nullptr */
};

class media_event_sink {
public:
    virtual ~media_event_sink() = default;
    virtual void post_event(const media_event &ev) = 0;
};

#endif /* __MEDIA_EVENTS_HPP__ */

