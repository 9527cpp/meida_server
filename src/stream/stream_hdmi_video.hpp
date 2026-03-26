#ifndef __STREAM_HDMI_VIDEO_HPP__
#define __STREAM_HDMI_VIDEO_HPP__

#include "stream_base.hpp"
#include "mpi_ctx/mpi_ctx_intf.h"
#include <memory>
#include <vector>
#include <map>

class stream_vpss;
class stream_venc;

/**
 * stream_hdmi_video: HDMI 视频流管理器
 *
 * 拓扑结构:
 *   stream_vi (唯一入口)
 *      |
 *      +--[直接]--> stream_venc[chn]  (无处理需求的输出)
 *      |
 *      +--[经过]--> stream_vpss[0] --> stream_venc[chn] (有处理需求的输出)
 *                  stream_vpss[1] --> stream_venc[chn]
 *                      ...
 *
 * VPSS 处理节点可复用：相同处理参数的多个输出共享同一个 VPSS
 */
class stream_hdmi_video : public stream_base {
public:
    stream_hdmi_video();
    ~stream_hdmi_video();

    /* 启动/停止（chn 指 venc 通道） */
    int start(int chn);
    int stop(int chn);

    /* 添加监听者（chn 指 venc 通道） */
    void add_listener(int chn, stream_listener *listener);
    void remove_listener(int chn, stream_listener *listener);

    /* 添加直接输出（无处理，venc 直接挂在 vi 上） */
    int add_direct_venc(int chn);

    /* 添加经过 VPSS 处理的输出（vpss_chn 指定 VPSS 通道） */
    int add_vpss_venc(int chn, int vpss_chn);

    /* 查找或创建 VPSS（根据处理需求 hash 或通道号） */
    int get_or_create_vpss(int vpss_chn);

    /* 获取 VENC 数量 */
    int get_venc_count() const { return (int)vencs_.size(); }

protected:
    void stream_data_loop() override;

private:
    void init_vi();
    void deinit_streams();

    stream_base *vi_{nullptr};
    std::vector<stream_base *> vencs_;
    std::vector<stream_base *> vpss_list_;
    std::map<int, stream_base *> vpss_map_;  // vpss_chn -> stream_vpss*
};

#endif /* __STREAM_HDMI_VIDEO_HPP__ */
