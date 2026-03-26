#include "stream_hdmi_video.hpp"
#include "stream_base.hpp"
#include "stream_vpss.hpp"
#include "log/log_tag.h"

#define MODULE_TAG "stream_hdmi_video"

stream_hdmi_video::stream_hdmi_video()
{
    init_vi();
}

stream_hdmi_video::~stream_hdmi_video()
{
    deinit_streams();
}

void stream_hdmi_video::init_vi()
{
    vi_ = new stream_vi();
}

void stream_hdmi_video::deinit_streams()
{
    for (auto *venc : vencs_) {
        delete venc;
    }
    vencs_.clear();

    for (auto *vpss : vpss_list_) {
        delete vpss;
    }
    vpss_list_.clear();
    vpss_map_.clear();

    if (vi_) {
        delete vi_;
        vi_ = nullptr;
    }
}

int stream_hdmi_video::start(int chn)
{
    if (chn < 0 || chn >= MAX_VENC_CHN)
        return -1;

    int ret = 0;

    /* 启动 VI */
    if (vi_ && vi_->get_status() != STREAM_STATUS_RUNNING) {
        ret = vi_->stream_start();
        if (ret != 0)
            return ret;
    }

    /* 启动对应通道的 VENC */
    if (chn < (int)vencs_.size() && vencs_[chn]) {
        ret = vencs_[chn]->stream_start();
    }

    return ret;
}

int stream_hdmi_video::stop(int chn)
{
    if (chn < 0 || chn >= MAX_VENC_CHN)
        return -1;

    int ret = 0;

    /* 停止对应通道的 VENC */
    if (chn < (int)vencs_.size() && vencs_[chn]) {
        ret = vencs_[chn]->stream_stop();
    }

    return ret;
}

void stream_hdmi_video::add_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_VENC_CHN)
        return;
    if (chn < (int)vencs_.size() && vencs_[chn] && listener) {
        vencs_[chn]->stream_add_listener(listener);
    }
}

void stream_hdmi_video::remove_listener(int chn, stream_listener *listener)
{
    if (chn < 0 || chn >= MAX_VENC_CHN)
        return;
    if (chn < (int)vencs_.size() && vencs_[chn] && listener) {
        vencs_[chn]->stream_del_listener(listener);
    }
}

int stream_hdmi_video::add_direct_venc(int chn)
{
    if (chn < 0 || chn >= MAX_VENC_CHN)
        return -1;

    /* 确保 vencs_ 数组有足够大小 */
    if (chn >= (int)vencs_.size()) {
        vencs_.resize(chn + 1, nullptr);
    }

    if (vencs_[chn])
        return 0;  // 已存在

    stream_base *venc = new stream_venc(chn);
    vencs_[chn] = venc;

    /* VENC 直接挂在 VI 下 */
    vi_->add_stream(venc);

    return 0;
}

int stream_hdmi_video::get_or_create_vpss(int vpss_chn)
{
    if (vpss_chn < 0 || vpss_chn >= MAX_VPSS_CHN)
        return -1;

    auto it = vpss_map_.find(vpss_chn);
    if (it != vpss_map_.end()) {
        return 0;  // VPSS 已存在
    }

    stream_vpss *vpss = new stream_vpss(vpss_chn);
    vpss_list_.push_back(vpss);
    vpss_map_[vpss_chn] = vpss;

    /* VPSS 挂在 VI 下 */
    vi_->add_stream(vpss);

    return 0;
}

int stream_hdmi_video::add_vpss_venc(int chn, int vpss_chn)
{
    if (chn < 0 || chn >= MAX_VENC_CHN || vpss_chn < 0 || vpss_chn >= MAX_VPSS_CHN)
        return -1;

    /* 确保 vencs_ 数组有足够大小 */
    if (chn >= (int)vencs_.size()) {
        vencs_.resize(chn + 1, nullptr);
    }

    if (vencs_[chn])
        return -1;  // 已存在

    /* 确保 VPSS 存在 */
    get_or_create_vpss(vpss_chn);

    stream_base *vpss = vpss_map_[vpss_chn];
    if (!vpss)
        return -1;

    stream_base *venc = new stream_venc(chn);
    vencs_[chn] = venc;

    /* VENC 挂在 VPSS 下 */
    vpss->add_stream(venc);

    return 0;
}

void stream_hdmi_video::stream_data_loop()
{
    while (is_running()) {
        usleep(100000);
    }
}
