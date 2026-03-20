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

extern struct mpi_intf stub_mpi;
extern struct mpi_intf rockit_mpi;
static volatile int g_running = 1;

static void signal_handler(int)
{
    g_running = 0;
}

int main(int argc, char *argv[])
{
    // must
    static struct mpi_intf mpi;
    struct mpi_ctx_intf *ctx_intf = nullptr;
    media_manager mgr;

    // option
    std::unique_ptr<uds_connection_manager> uds_mgr;
    std::unique_ptr<file_stream> file_out;
    const char *uds_path = "/tmp/media_server.sock";
    const char *file_path = "/tmp/media_server.out";

    // 注册信号处理函数
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 设置 mpi, 加载 mpi 配置
    mpi = rockit_mpi;
    ctx_intf = mpi_ctx_intf_json_create("/etc/media_server.json");
    if (!ctx_intf) ctx_intf = mpi_ctx_intf_json_create(nullptr);

    mpi_intf_set_ctx(&mpi, ctx_intf); // 设置 mpi 的 ctx
    mpi_intf_register(&mpi); // 注册 mpi, 从而 media_manager 可以获取 mpi 实例

    // 初始化 media_manager, 创建 mpi 各个通道, 启用事件循环(接收如uds来的启用停止流的通知)
    if (mgr.init() != 0) {
        fprintf(stderr, "media_manager init failed\n");
        return 1;
    }

    // 初始化 uds_connection_manager
    uds_mgr.reset(new uds_connection_manager(&mgr, uds_path));
    if (uds_mgr->start() != 0) {
        fprintf(stderr, "uds_connection_manager start failed\n");
        mgr.deinit();
        return 1;
    }

    // 主循环, 可用于 主线程接收一些退出事件
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // 停止 uds_connection_manager
    if (uds_mgr) uds_mgr->stop();

    // 释放资源
    mgr.deinit();
    fprintf(stderr, "[main] exit\n");
    return 0;
}
