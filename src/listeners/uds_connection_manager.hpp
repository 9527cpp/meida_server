#ifndef __UDS_CONNECTION_MANAGER_HPP__
#define __UDS_CONNECTION_MANAGER_HPP__

#include "uds_stream.hpp"
#include "media/media_events.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>

/**
 * UDS 连接管理：每个客户端连接自动分配一个未占用通道(chn), 并添加一个uds的数据回调，用于将数据发送到客户端。
 * 协议：
 *  1) 优先支持 libuds 命令包（带 cmd + json payload）；
 *  2) 为兼容历史客户端，仍支持 1 字节媒体类型：
 *     - video: 0/'v'/'V'
 *     - audio: 1/'a'/'A'
 * 若为 video，启动对应 chn 的 hdmi_video 并把该 chn 数据经 socket 发给客户端。
 * 若为 audio，启动 hdmi_audio。
 */
class uds_connection_manager {
public:
    uds_connection_manager(media_event_sink *sink, const std::string &uds_path);
    ~uds_connection_manager();

    int start();
    void stop();

private:
    enum class media_type {
        video = 0,
        audio = 1
    };

    struct session {
        int fd;
        int chn;
        media_type type;
        std::unique_ptr<uds_stream> stream;
        session(int f, int c, media_type t, uds_stream *s) : fd(f), chn(c), type(t), stream(s) {}
    };

    struct connect_request {
        media_type type;
        int cmd;
        std::string json_payload;
    };

    void accept_loop();
    void cleanup_loop();
    static bool parse_connect_request(int fd, int timeout_ms, connect_request *out_req);
    static bool parse_media_type_from_cmd(int cmd, media_type *out_type);
    static const char *cmd_name(int cmd);
    int alloc_free_channel_locked(media_type type) const;

    media_event_sink *sink_;
    std::string uds_path_;
    int listen_fd_;
    std::atomic<bool> running_;
    std::vector<session> sessions_;
    std::mutex sessions_mutex_;
    std::thread accept_thread_;
    std::thread cleanup_thread_;
};

#endif /* __UDS_CONNECTION_MANAGER_HPP__ */
