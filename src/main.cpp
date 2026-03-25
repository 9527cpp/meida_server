/*
 * media-server 主程序：MPI 注册、ctx 配置、media_manager + 可选 UDS / 文件输出
 *
 * 默认：不创建 UDS、不写文件，仅初始化 media_manager 后空转（可用于纯 MPI/检测场景）。
 *
 * 参数：
 *   --mpi <path>   指定平台 MPI 动态库路径（如 libmpi_rockit.so）
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
#include <dlfcn.h>
#include <getopt.h>

#define MODULE_TAG "main"

#include "log/log_tag.h"

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
    printf("Usage: %s [OPTIONS]\n"
           "  -m, --mpi <path>    MPI shared library path (e.g. libmpi_rockit.so)\n"
           "  -c, --cfg <path>    Config file path (default: /tmp/config.json)\n"
           "  -u, --enable-uds    Enable UDS server (multi-client)\n"
           "  -f, --enable-file   Write channel 0 encoded video to a file\n"
           "  -h, --help          Show this help\n"
           "\n"
           "Options -u and -f are mutually exclusive.\n",
           prog);
}

static const struct option long_options[] = {
    {"mpi",         required_argument, NULL, 'm'},
    {"cfg",         required_argument, NULL, 'c'},
    {"enable-uds",  no_argument,       NULL, 'u'},
    {"enable-file", no_argument,       NULL, 'f'},
    {"help",        no_argument,       NULL, 'h'},
    {NULL,          0,                 NULL,  0 },
};

int main(int argc, char *argv[])
{
    media_manager mgr;
    std::unique_ptr<uds_connection_manager> uds_mgr;
    std::unique_ptr<file_stream> file_out;
    const char *uds_path = "/tmp/media_server.sock";
    const char *file_path = "/tmp/media_server.out";
    const char *cfg_path = "/tmp/config.json";
    const char *mpi_so_path = NULL;

    struct mpi_ctx_intf *ctx_intf = &json_ctx_intf;
    bool enable_uds = false;
    bool enable_file = false;

    int opt;
    while ((opt = getopt_long(argc, argv, "m:c:ufh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'm':
            mpi_so_path = optarg;
            break;
        case 'c':
            cfg_path = optarg;
            break;
        case 'u':
            enable_uds = true;
            break;
        case 'f':
            enable_file = true;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (enable_uds && enable_file) {
        WriteLog(LOG_ERROR, "--enable-uds and --enable-file cannot be used together.");
        print_usage(argv[0]);
        return 1;
    }

    if (!mpi_so_path) {
        WriteLog(LOG_ERROR, "--mpi <path> is required (e.g. --mpi ./libmpi_stub.so)");
        print_usage(argv[0]);
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    void *mpi_handle = dlopen(mpi_so_path, RTLD_NOW);
    if (!mpi_handle) {
        WriteLog(LOG_ERROR, "dlopen(%s) failed: %s", mpi_so_path, dlerror());
        return 1;
    }

    mpi_intf_create_fn create_fn =
        (mpi_intf_create_fn)dlsym(mpi_handle, MPI_INTF_CREATE_SYMBOL);
    if (!create_fn) {
        WriteLog(LOG_ERROR, "dlsym(%s) failed: %s", MPI_INTF_CREATE_SYMBOL, dlerror());
        dlclose(mpi_handle);
        return 1;
    }

    struct mpi_intf *mpi = create_fn();
    if (!mpi) {
        WriteLog(LOG_ERROR, "mpi_intf_create() returned NULL");
        dlclose(mpi_handle);
        return 1;
    }
    WriteLog(LOG_INFO, "loaded MPI from %s", mpi_so_path);

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

    if (mpi_handle) {
        dlclose(mpi_handle);
        mpi_handle = NULL;
    }

    WriteLog(LOG_INFO, "exit");
    return 0;
}
