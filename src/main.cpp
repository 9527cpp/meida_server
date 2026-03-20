/*
 * media-server 主程序：MPI 注册、ctx 配置、media_manager + 可选 UDS / 文件输出
 *
 * 默认：不创建 UDS、不写文件，仅初始化 media_manager 后空转（可用于纯 MPI/检测场景）。
 *
 * 参数：
 *   --enable-uds   启用 Unix socket 多客户端（默认 /tmp/media_server.sock）
 *   --enable-file  将通道 0 视频码流写入文件（默认 /tmp/media_server.out）
 *   -h, --help     打印帮助
 *
 * 注意：--enable-uds 与 --enable-file 不能同时使用（会争用同一路 hdmi 视频管道）。
 */
#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"
#include "media/media_manager.hpp"
#include "media/media_events.hpp"
#include "listeners/file_stream.hpp"
#include "listeners/uds_connection_manager.hpp"

#include <cstdio>
#include <cstring>
#include <csignal>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

// module log tag
#define MODULE_TAG "main"

#include "log/log_tag.h"

extern struct mpi_intf rockit_mpi;
extern struct mpi_ctx_intf json_ctx_intf;
extern "C" {
    extern struct mpi_ctx *default_ctx(void);
}

static volatile int g_running = 1;

static void signal_handler(int)
{
    g_running = 0;
}

static void print_usage(const char *prog)
{
    WriteLog(LOG_INFO,
              "Usage: %s [OPTIONS]"
              "  --enable-uds    Enable UDS server (multi-client)"
              "  --enable-file   Write channel 0 encoded video to a file"
              "  -h, --help      Show this help"
              "Options --enable-uds and --enable-file are mutually exclusive.",
              prog);
}

int main(int argc, char *argv[])
{
    media_manager mgr;
    std::unique_ptr<uds_connection_manager> uds_mgr;
    std::unique_ptr<file_stream> file_out;
    const char *uds_path = "/tmp/media_server.sock";
    const char *file_path = "/tmp/media_server.out";
    char cfg_path[256] = "/tmp/config.json";

    // 可替换为其它 MPI 实现
    static struct mpi_intf *mpi = &rockit_mpi;
    // 可替换为其它 ctx 实现
    struct mpi_ctx_intf *ctx_intf = &json_ctx_intf;
    bool enable_uds = false;
    bool enable_file = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--enable-uds") == 0) {
            enable_uds = true;
        } else if (strcmp(argv[i], "--enable-file") == 0) {
            enable_file = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--cfg-path") == 0) {
            if (i + 1 < argc) {
                strncpy(cfg_path, argv[i + 1], sizeof(cfg_path) - 1);
                i++;
            } else {
                WriteLog(LOG_ERROR, "--cfg-path requires an argument");
                print_usage(argv[0]);
                return 1;
            }
        } else {
            WriteLog(LOG_ERROR, "Unknown option: %s", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (enable_uds && enable_file) {
        WriteLog(LOG_ERROR, "Error: --enable-uds and --enable-file cannot be used together.");
        print_usage(argv[0]);
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    struct mpi_ctx *ctx_data = ctx_intf->load(cfg_path);
    if (!ctx_data) {
        WriteLog(LOG_ERROR, "mpi_ctx load %s failed, use default config !!!", cfg_path);
        ctx_data = default_ctx();
    }

    mpi_intf_init(mpi, ctx_data);

    mpi_intf_register(mpi);

    if (enable_uds) {
        uds_mgr.reset(new uds_connection_manager(&mgr, uds_path));
        if (uds_mgr->start() != 0) {
            WriteLog(LOG_ERROR, "uds_connection_manager start failed");
            mgr.deinit();
            return 1;
        }
        WriteLog(LOG_INFO, "UDS enabled: %s", uds_path);
    } else if (enable_file) {
        file_out.reset(new file_stream(file_path));
        media_event ev_start;
        ev_start.type = media_event_type::start_video;
        ev_start.chn = 0;
        ev_start.listener = file_out.get();
        mgr.post_event(ev_start);
        WriteLog(LOG_INFO, "file output enabled: %s (chn=0)", file_path);
    } else {
        WriteLog(LOG_WARNING, "no UDS / file mode (use --enable-uds or --enable-file)");
    }

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (uds_mgr)
        uds_mgr->stop();

    if (file_out) {
        media_event ev_stop;
        ev_stop.type = media_event_type::stop_video;
        ev_stop.chn = 0;
        ev_stop.listener = file_out.get();
        mgr.post_event(ev_stop);
        /* 给 event_loop 时间处理 stop，再 deinit 关闭线程 */
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (ctx_data) {
        ctx_intf->free_ctx(ctx_data);
    }

    mpi_intf_deinit(mpi);

    WriteLog(LOG_INFO, "exit");
    return 0;
}
