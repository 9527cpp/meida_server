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
    extern struct mpi_intf *mpi_intf_create(void);
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
    const char *uds_path = "/tmp/media_server.sock";
    const char *cfg_path = nullptr;
    const char *mpi_so_path = NULL;

    struct mpi_ctx_intf *ctx_intf = &json_ctx_intf;
    bool enable_uds = false;

    int opt;
    while ((opt = getopt_long(argc, argv, "m:c:uh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'm':
            mpi_so_path = optarg;
            break;
        case 'c':
            cfg_path = strdup(optarg);
            break;
        case 'u':
            enable_uds = true;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 1: 加载 MPI的so 以及注册 mpi 实例 */
    void *mpi_handle = NULL;
    struct mpi_intf *mpi = NULL;

    if (mpi_so_path) {
        mpi_handle = dlopen(mpi_so_path, RTLD_NOW);
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
        mpi = create_fn();
        if (!mpi) {
            WriteLog(LOG_ERROR, "mpi_intf_create() returned NULL");
            dlclose(mpi_handle);
            return 1;
        }
        WriteLog(LOG_INFO, "loaded MPI from %s", mpi_so_path);
    } else {
        /* 使用 stub MPI */
        mpi = mpi_intf_create();
        if (!mpi) {
            WriteLog(LOG_ERROR, "mpi_intf_create() returned NULL");
            return 1;
        }
        WriteLog(LOG_INFO, "using stub MPI");
    }
    mpi_intf_register(mpi);

    /* 2: 加载配置, 并绑定到 mpi 实例 */
    struct mpi_ctx *ctx_data = nullptr;
    if (cfg_path) {
        ctx_data = ctx_intf->load(cfg_path);
        if (!ctx_data) {
            WriteLog(LOG_ERROR, "mpi_ctx load %s failed, use default config !!!", cfg_path);
            return -1;
        }
    } else {
        ctx_data = default_ctx();
    }
    mpi_intf_init(mpi, ctx_data);

    /* 是否启用 UDS 服务, 启动 UDS 服务 */
    if (enable_uds) {
        uds_mgr.reset(new uds_connection_manager(&mgr, uds_path));
        if (uds_mgr->start() != 0) {
            WriteLog(LOG_ERROR, "uds_connection_manager start failed");
            mgr.deinit();
            return 1;
        }
        WriteLog(LOG_INFO, "UDS enabled: %s", uds_path);
    }

    /* 主循环, 等待退出信号触发 */
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    /* 停止 UDS 服务 */
    if (enable_uds) {
        uds_mgr->stop();
    }

    /* 释放配置 */
    if (ctx_data) {
        ctx_intf->free_ctx(ctx_data);
    }

    /* 释放 mpi 实例 */
    if (mpi) {
        mpi_intf_deinit(mpi);
        mpi = NULL;
    }

    /* 释放 MPI 的 so */
    if (mpi_handle) {
        dlclose(mpi_handle);
        mpi_handle = NULL;
    }

    WriteLog(LOG_INFO, "exit");

    return 0;
}
