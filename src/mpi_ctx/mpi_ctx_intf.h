#ifndef __MPI_CTX_INTF_H__
#define __MPI_CTX_INTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

/*
    TODO: 暂时写的都是4路，后续根据实际需求再调整
 */
#define MAX_VI_CHN 4 
#define MAX_VPSS_CHN 4 
#define MAX_VENC_CHN 4 
#define MAX_AI_CHN 4 
#define MAX_AENC_CHN 4 

/* VI 上下文 */
struct vi_ctx {
    int devid;
    int pipeid;
    int chn_index;
    char * entity_name;
    int format;
    int width;
    int height;
    int buf_cnt;
    char * out_file;
    int memory_type;
    bool enable;
    int fps;
};

/* VPSS 上下文（视频处理：缩放、水印等） */
struct vpss_ctx {
    int chn_index;
    int width;
    int height;
    int out_width;
    int out_height;
    bool enable;
};

/* VENC 上下文 */
struct venc_ctx {
    int chn_index;
    char * type; // h264 ? h265 ? mjpeg
    bool enable;

    struct venc_attr_ {
        int pixel_format;
        int width;
        int height;
        int vir_width;
        int vir_height;
        int profile;
        int stream_buf_cnt;
    }venc_attr;

    struct rc_attr_ {
        char * mode; // cbr ? vbr ? fixqp
        int gop;
        int bitrate;

        int src_frame_rate_num;
        int src_frame_rate_den;
        int dst_frame_rate_den;
        float min_bitrate_roti;
        float max_bitrate_roti;

        int start_qp; 
        int step_qp;
        int delt_ip_qp;
        int min_qp;
        int max_qp;
        int min_iqp;
        int max_iqp;
        int frm_min_qp;
        int frm_max_qp;
        int frm_min_iqp;
        int frm_max_iqp;
        int max_re_encode_times;
    }rc_attr;

    int full_range_flag;
};

/* AI 上下文 */
struct ai_ctx {
    int chn;
    int channel;
    int samprate;
    int bit;
};

/* AENC 上下文 */
struct aenc_ctx {
    int chn;
    int buf_time;
    int period;
};

/* 所有的视频配置 */
struct video_ctx {
    struct vi_ctx vi[MAX_VI_CHN];
    int vi_cfg_count; /* how many vi entries are parsed from config (logical index 0..vi_cfg_count-1) */
    struct vpss_ctx vpss[MAX_VPSS_CHN];
    int vpss_cfg_count; /* how many vpss entries are parsed from config (logical index 0..vpss_cfg_count-1) */
    struct venc_ctx venc[MAX_VENC_CHN];
    int venc_cfg_count; /* how many venc entries are parsed from config (logical index 0..venc_cfg_count-1) */
};

/* 所有的音频配置 */
struct audio_ctx {
    struct ai_ctx ai[MAX_AI_CHN];
    int ai_cfg_count; /* how many ai entries are parsed from config (logical index 0..ai_cfg_count-1) */
    struct aenc_ctx aenc[MAX_AENC_CHN];
    int aenc_cfg_count; /* how many aenc entries are parsed from config (logical index 0..aenc_cfg_count-1) */
};

/* MPI 所有配置（抽象接口，由 json_cfg/uci_cfg 等实现填充） */
struct mpi_ctx {
    struct video_ctx v_ctx;
    struct audio_ctx a_ctx;
};

/* 上下文接口：从配置源加载/解析得到 mpi_ctx */
struct mpi_ctx_intf {
    struct mpi_ctx *(*load)(const char *cfg_path);
    void (*free_ctx)(struct mpi_ctx *ctx);
};

#ifdef __cplusplus
}
#endif

#endif /* __MPI_CTX_INTF_H__ */
