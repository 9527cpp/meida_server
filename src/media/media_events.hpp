#ifndef __MEDIA_EVENTS_HPP__
#define __MEDIA_EVENTS_HPP__

#include "../stream/stream_listener.hpp"

enum class media_event_type {
    start_video,
    stop_video,
    start_audio,
    stop_audio,
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

