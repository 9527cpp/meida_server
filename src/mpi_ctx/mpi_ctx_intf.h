#ifndef __MPI_CTX_INTF_H__
#define __MPI_CTX_INTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_CHN 4

/* 系统上下文 */
struct sys_ctx {
    int devid;
    int pipeid;
};

/* VI 上下文 */
struct vi_ctx {
    int chn;
    int buf_cnt;
};

/* 视频编码上下文 */
struct venc_ctx {
    int chn;
    int bitrate;
    int frame_rate;
    int entype;
    int buf_cnt;
};

/* 音频输入上下文 */
struct ai_ctx {
    int chn;
    int channel;
    int samprate;
    int bit;
};

/* 音频编码上下文 */
struct aenc_ctx {
    int chn;
    int buf_time;
    int period;
};

/* 视频管道上下文：一个 VI 后接多个 VENC */
struct video_ctx {
    struct vi_ctx vi;
    struct venc_ctx venc[MAX_CHN];
};

/* 音频管道上下文 */
struct audio_ctx {
    struct ai_ctx ai;
    struct aenc_ctx aenc[MAX_CHN];
};

/* MPI 完整上下文（抽象接口，由 json_cfg/uci_cfg 等实现填充） */
struct mpi_ctx {
    struct sys_ctx s_ctx;
    struct video_ctx v_ctx;
    struct audio_ctx a_ctx;
};

/* 上下文接口：从配置源加载/解析得到 mpi_ctx */
struct mpi_ctx_intf {
    struct mpi_ctx *(*load)(void);
    void (*free_ctx)(struct mpi_ctx *ctx);
};

/* 创建基于 JSON 的上下文实现 */
struct mpi_ctx_intf *mpi_ctx_intf_json_create(const char *cfg_path);

/* 创建基于 UCI 的上下文实现 */
struct mpi_ctx_intf *mpi_ctx_intf_uci_create(void);

#ifdef __cplusplus
}
#endif

#endif /* __MPI_CTX_INTF_H__ */
