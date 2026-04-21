#include "media_manager.hpp"
#define MODULE_TAG "media_manager"

#include "log/log_tag.h"

#include <dirent.h>
#include <cstring>
#include <dlfcn.h>

/* 静态全局回调桥接（用于将 C 回调转发到 media_manager 成员函数） */
static media_manager *g_bridge_mgr = nullptr;

media_manager::media_manager(const char *hw_plugin_dir)
    : ev_running_(false)
{
    g_bridge_mgr = this;

    hdmi_video_pipe_.reset(new stream_hdmi_video());
    hdmi_audio_pipe_.reset(new stream_hdmi_audio());
    // usb_video_pipe_.reset(new stream_usb_video());
    // mic_audio_pipe_.reset(new stream_mic_audio());

    if (hw_plugin_dir) {
        load_hw_check_plugins(hw_plugin_dir);
    }

    init();
}

media_manager::~media_manager()
{
    deinit();
}

int media_manager::init()
{
    {
        std::lock_guard<std::mutex> lock(ev_mutex_);
        ev_running_ = true;
    }
    ev_thread_ = std::thread(&media_manager::event_loop, this);
    return 0;
}

void media_manager::deinit()
{
    unload_hw_check_plugins();

    {
        std::lock_guard<std::mutex> lock(ev_mutex_);
        if (ev_running_) {
            ev_running_ = false;
            ev_cv_.notify_all();
        }
    }
    if (ev_thread_.joinable())
        ev_thread_.join();
}


void media_manager::post_event(const media_event &ev)
{
    {
        std::lock_guard<std::mutex> lock(ev_mutex_);
        if (!ev_running_)
            return;
        ev_q_.push(ev);
    }
    ev_cv_.notify_one();
}

/*
    目前有两个地方会发事件
    1: hw_check 事件回调, 收到事件后 发消息给event_loop
    2: uds 连接管理, 收到连接请求后 发消息给event_loop
    从 队列中 取事件, 处理事件: 如启用/停止 视频/音频 流
*/
void media_manager::event_loop()
{
    while (true) {
        media_event ev;
        {
            std::unique_lock<std::mutex> lock(ev_mutex_);
            ev_cv_.wait(lock, [this]() { return !ev_running_ || !ev_q_.empty(); });
            if (!ev_running_ && ev_q_.empty())
                break;
            ev = ev_q_.front();
            ev_q_.pop();
        }

        if (ev.chn < 0 || ev.chn >= MAX_VENC_CHN) {
            WriteLog(LOG_ERROR, "invalid channel: chn=%d", ev.chn);
            continue;
        }

        switch (ev.type) {
        case media_event_type::start_video:
            hdmi_video_pipe_->start(ev.chn);
            break;
        case media_event_type::stop_video:
            hdmi_video_pipe_->stop(ev.chn);
            break;
        case media_event_type::assign_video:
            hdmi_video_pipe_->add_listener(ev.chn, ev.listener);
            break;
        case media_event_type::unassign_video:
            hdmi_video_pipe_->remove_listener(ev.chn, ev.listener);
            break;
        case media_event_type::start_audio:
            hdmi_audio_pipe_->start(ev.chn);
            break;
        case media_event_type::stop_audio:
            hdmi_audio_pipe_->stop(ev.chn);
            break;
        case media_event_type::assign_audio:
            hdmi_audio_pipe_->add_listener(ev.chn, ev.listener);
            break;
        case media_event_type::unassign_audio:
            hdmi_audio_pipe_->remove_listener(ev.chn, ev.listener);
            break;
        /* TODO: 其他事件处理 */
        }
    }
}

/*
    hw_check C 回调桥接函数（全局静态，由 .so 调用）
    将 C 回调转发到 media_manager 的成员函数
*/
static void hw_check_bridge_cbk(enum hw_check_type type,
                                  enum hw_check_event ev,
                                  void *user_data)
{
    (void)user_data;
    if (g_bridge_mgr) {
        g_bridge_mgr->on_hw_check_notify(type, ev);
    }
}

/*
    扫描目录，加载所有 libhw_check_*.so
*/
int media_manager::load_hw_check_plugins(const char *dir)
{
    DIR *d = opendir(dir);
    if (!d) {
        WriteLog(LOG_ERROR, "opendir(%s) failed: %s", dir, strerror(errno));
        return -1;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != nullptr) {
        if (strncmp(ent->d_name, "libhw_check_", 12) != 0)
            continue;
        if (!strstr(ent->d_name, ".so"))
            continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);

        void *handle = dlopen(path, RTLD_NOW);
        if (!handle) {
            WriteLog(LOG_ERROR, "dlopen(%s) failed: %s", path, dlerror());
            continue;
        }

        hw_check_create_fn create_fn =
            (hw_check_create_fn)dlsym(handle, HW_CHECK_CREATE_SYMBOL);
        if (!create_fn) {
            WriteLog(LOG_ERROR, "dlsym(%s, %s) failed: %s",
                     path, HW_CHECK_CREATE_SYMBOL, dlerror());
            dlclose(handle);
            continue;
        }

        struct hw_check_intf *intf = create_fn();
        if (!intf) {
            WriteLog(LOG_ERROR, "hw_check_intf_create() returned NULL for %s", path);
            dlclose(handle);
            continue;
        }

        hw_check_instance inst = {};
        inst.so_handle = handle;
        inst.opt = intf->opt;
        inst.ctx = intf->opt->create(
            HW_TYPE_HDMI, hw_check_bridge_cbk, this, true);

        hw_check_instances_.push_back(inst);
        WriteLog(LOG_INFO, "loaded hw_check plugin: %s (%s)",
                 path, intf->name ? intf->name : "unknown");
    }

    closedir(d);
    return 0;
}

void media_manager::unload_hw_check_plugins()
{
    for (auto &inst : hw_check_instances_) {
        if (inst.ctx && inst.opt) {
            inst.opt->destroy(inst.ctx);
        }
        if (inst.so_handle) {
            dlclose(inst.so_handle);
        }
    }
    hw_check_instances_.clear();
}

/*
    hw_check 事件回调, 收到事件后 发消息给event_loop, 让event_loop去处理对应的流事件
*/
void media_manager::on_hw_check_notify(enum hw_check_type type, enum hw_check_event ev)
{
    const char *t = "unknown";
    switch (type) {
    case HW_TYPE_HDMI: t = "hdmi"; break;
    case HW_TYPE_USB:  t = "usb";  break;
    case HW_TYPE_MIC:  t = "mic";  break;
    }

    const char *e = (ev == HW_EV_PLUG_IN) ? "plug_in" : "plug_out";
    WriteLog(LOG_INFO, "%s %s", t, e);

    if (type == HW_TYPE_HDMI) {
        /* HDMI plug_in/out：对所有 enable 的 venc 通道分别 start/stop */
        for (int i = 0; i < MAX_VENC_CHN; i++) {
            media_event ev_media;
            ev_media.type = (ev == HW_EV_PLUG_IN) ? media_event_type::start_video
                                                 : media_event_type::stop_video;
            ev_media.chn = i;
            ev_media.listener = nullptr;
            post_event(ev_media);
        }
        return;
    }

    /* TODO: 其它硬件类型处理 */
}
