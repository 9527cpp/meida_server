/*
 * media-server 主程序：演示 MPI 注册、ctx 配置、media_manager + 监听者、按 hw_check 启停流
 * 使用方式 1：平台 MPI 实现并注册，ctx 用 json/uci 加载并设置
 * 使用方式 2：创建 media_manager，设置 listener（uds_stream/file_stream），init 后按需启停通道
 * 使用方式 3：UDS 服务端 accept 后 set_client_fd；按 hw_check 结果启停流
 */
#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"
#include "media/media_manager.hpp"
#include "listeners/uds_stream.hpp"
#include "listeners/file_stream.hpp"
#include "listeners/uds_connection_manager.hpp"

#include <cstdio>
#include <cstring>
#include <csignal>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

static volatile int g_running = 1;

static void signal_handler(int)
{
    g_running = 0;
}

/* ========== 1. 平台侧 MPI 接口实现（桩实现，实际由各平台替换） ========== */
extern "C" {

static int stub_sys_init(void)
{
    fprintf(stderr, "[mpi] sys_init\n");
    return 0;
}

static int stub_sys_deinit(void)
{
    fprintf(stderr, "[mpi] sys_deinit\n");
    return 0;
}

static int stub_vi_init(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] vi_init\n");
    return 0;
}

static int stub_vi_deinit(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] vi_deinit\n");
    return 0;
}

static int stub_vpss_init(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] vpss_init\n");
    return 0;
}

static int stub_vpss_deinit(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] vpss_deinit\n");
    return 0;
}

static int stub_venc_init(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] venc_init\n");
    return 0;
}

static int stub_venc_deinit(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] venc_deinit\n");
    return 0;
}

static int stub_venc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    /* 桩：模拟无数据，返回 -1 让 stream_venc 线程 sleep；实际平台从编码器取数 */
    (void)data;
    (void)len;
    return -1;
}

static int stub_ai_init(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] ai_init\n");
    return 0;
}

static int stub_ai_deinit(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] ai_deinit\n");
    return 0;
}

static int stub_aenc_init(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] aenc_init\n");
    return 0;
}

static int stub_aenc_deinit(void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[mpi] aenc_deinit\n");
    return 0;
}

static int stub_aenc_get_data(void *ctx, void *data, int *len)
{
    (void)ctx;
    (void)data;
    (void)len;
    return -1;
}

} /* extern "C" */

static void setup_mpi_and_ctx(struct mpi_intf *mpi,
                              struct mpi_sys_opt *sys_opt,
                              struct mpi_video_opt *video_opt,
                              struct mpi_audio_opt *audio_opt,
                              struct mpi_ctx_intf **out_ctx_intf)
{
    sys_opt->sys_init = stub_sys_init;
    sys_opt->sys_deinit = stub_sys_deinit;

    video_opt->vi_init = stub_vi_init;
    video_opt->vi_deinit = stub_vi_deinit;
    video_opt->vpss_init = stub_vpss_init;
    video_opt->vpss_deinit = stub_vpss_deinit;
    video_opt->venc_init = stub_venc_init;
    video_opt->venc_deinit = stub_venc_deinit;
    video_opt->venc_get_data = stub_venc_get_data;

    audio_opt->ai_init = stub_ai_init;
    audio_opt->ai_deinit = stub_ai_deinit;
    audio_opt->aenc_init = stub_aenc_init;
    audio_opt->aenc_deinit = stub_aenc_deinit;
    audio_opt->aenc_get_data = stub_aenc_get_data;

    mpi->sys = sys_opt;
    mpi->video = video_opt;
    mpi->audio = audio_opt;
    mpi->ctx = nullptr;
    mpi->ctx_data = nullptr;

    /* 使用方式 1：从 JSON 或 UCI 加载 ctx，并设置给 mpi */
    struct mpi_ctx_intf *ctx_intf = mpi_ctx_intf_json_create("/etc/media_server.json");
    if (!ctx_intf)
        ctx_intf = mpi_ctx_intf_json_create(nullptr); /* 无路径时用默认配置 */
    mpi_intf_set_ctx(mpi, ctx_intf);
    *out_ctx_intf = ctx_intf;

    mpi_intf_register(mpi);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 静态分配 MPI 相关结构，避免 C/C++ 混用时的生命周期问题 */
    static struct mpi_intf mpi;
    static struct mpi_sys_opt sys_opt;
    static struct mpi_video_opt video_opt;
    static struct mpi_audio_opt audio_opt;
    struct mpi_ctx_intf *ctx_intf = nullptr;

    /* ---------- 1. 平台 MPI 实现并注册，ctx 用 json 加载并设置 ---------- */
    setup_mpi_and_ctx(&mpi, &sys_opt, &video_opt, &audio_opt, &ctx_intf);
    (void)ctx_intf;

    /* ---------- 2. 创建 media_manager，init ---------- */
    media_manager mgr;
    const char *uds_path = "/tmp/media_server.sock";
    const char *file_path = "/tmp/media_server.out";
    bool use_uds_multi = true;  /* 默认：多客户端 UDS，每连接绑定一个 chn */

    if (argc >= 2 && strcmp(argv[1], "file") == 0)
        use_uds_multi = false;

    if (mgr.init() != 0) {
        fprintf(stderr, "media_manager init failed\n");
        return 1;
    }

    std::unique_ptr<uds_connection_manager> uds_mgr;
    std::unique_ptr<file_stream> file_out;

    if (use_uds_multi) {
        /* ---------- 3. UDS 多客户端：每个连接发 1 字节选通道(0=H264/1=H265/…)，该路 VENC 数据经此 socket 发给该客户端 ---------- */
        uds_mgr.reset(new uds_connection_manager(&mgr, uds_path));
        if (uds_mgr->start() != 0) {
            fprintf(stderr, "uds_connection_manager start failed\n");
            mgr.deinit();
            return 1;
        }
    } else {
        /* 单路写文件：仅通道 0 写入 file_path */
        file_out.reset(new file_stream(file_path));
        mgr.set_listener(file_out.get());
        //mgr.start_hdmi_video_channel(0);
    }

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (uds_mgr)
        uds_mgr->stop();
    // else
    //     mgr.stop_hdmi_video_channel(0);
    mgr.deinit();
    fprintf(stderr, "[main] exit\n");
    return 0;
}
