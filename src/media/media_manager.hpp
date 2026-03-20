#ifndef __MEDIA_MANAGER_HPP__
#define __MEDIA_MANAGER_HPP__

#include "../stream/stream_pipe.hpp"
#include "../stream/stream_listener.hpp"
#include "../hw_check/hw_check.hpp"
#include "../hw_check/hdmi_check.hpp"
#include "../hw_check/usb_check.hpp"
#include "media_events.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

/* 全局 media 管理：MPI/ctx 单例、各路流、监听者、硬件检测 */
class media_manager : public media_event_sink {
public:
    media_manager();
    ~media_manager();

    /* 初始化：需在设置 mpi/ctx 后调用 */
    int init();
    void deinit();

    /* 流启停 */
    int stream_start(int chn);
    int stream_stop(int chn);

    void post_event(const media_event &ev) override;

private:
    void event_loop();

    std::unique_ptr<stream_pipe> hdmi_video_pipe_;
    std::unique_ptr<stream_pipe> hdmi_audio_pipe_;
    std::unique_ptr<stream_pipe> usb_video_pipe_;
    std::unique_ptr<stream_pipe> mic_audio_pipe_;

    hw_check *hdmi_check_;
    hw_check *usb_check_;

    std::mutex ev_mutex_;
    std::condition_variable ev_cv_;
    std::queue<media_event> ev_q_;
    bool ev_running_;
    std::thread ev_thread_;
};

#endif /* __MEDIA_MANAGER_HPP__ */
