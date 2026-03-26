/*
 * MPI 上下文配置接口实现：json_cfg / uci_cfg
 */
#include "mpi_ctx_intf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct mpi_ctx *default_ctx(void)
{
    struct mpi_ctx *ctx = (struct mpi_ctx *)calloc(1, sizeof(struct mpi_ctx));
    if (!ctx)
        return NULL;

    /* VI 默认：1920x1080@25fps, channel0, enable */
    ctx->v_ctx.vi[0].devid = 0;
    ctx->v_ctx.vi[0].pipeid = 0;
    ctx->v_ctx.vi[0].chn_index = 0;
    ctx->v_ctx.vi[0].entity_name = NULL;
    ctx->v_ctx.vi[0].format = 0;
    ctx->v_ctx.vi[0].width = 1920;
    ctx->v_ctx.vi[0].height = 1080;
    ctx->v_ctx.vi[0].buf_cnt = 3;
    ctx->v_ctx.vi[0].out_file = NULL;
    ctx->v_ctx.vi[0].memory_type = 4; /* VI_V4L2_MEMORY_TYPE_MMAP (historical default) */
    ctx->v_ctx.vi[0].enable = true;
    ctx->v_ctx.vi[0].fps = 25;
    ctx->v_ctx.vi_cfg_count = 1;

    /* VENC 默认：字段尽量对齐 /tmp/config.json */
    for (int i = 0; i < MAX_VENC_CHN; i++) {
        ctx->v_ctx.venc[i].chn_index = i;
        ctx->v_ctx.venc[i].type = strdup("h264");
        ctx->v_ctx.venc[i].enable = true;

        ctx->v_ctx.venc[i].venc_attr.pixel_format = 0; /* YUV420SP */
        ctx->v_ctx.venc[i].venc_attr.width = 1920;
        ctx->v_ctx.venc[i].venc_attr.height = 1080;
        ctx->v_ctx.venc[i].venc_attr.vir_width = 1920;
        ctx->v_ctx.venc[i].venc_attr.vir_height = 1080;
        ctx->v_ctx.venc[i].venc_attr.profile = 77;
        ctx->v_ctx.venc[i].venc_attr.stream_buf_cnt = 3;

        ctx->v_ctx.venc[i].rc_attr.mode = strdup("cbr");
        ctx->v_ctx.venc[i].rc_attr.gop = 8000;
        ctx->v_ctx.venc[i].rc_attr.bitrate = 2000; /* kbps */
        ctx->v_ctx.venc[i].rc_attr.src_frame_rate_num = -1;
        ctx->v_ctx.venc[i].rc_attr.src_frame_rate_den = 1;
        ctx->v_ctx.venc[i].rc_attr.dst_frame_rate_den = 1;
        ctx->v_ctx.venc[i].rc_attr.min_bitrate_roti = 0.5f;
        ctx->v_ctx.venc[i].rc_attr.max_bitrate_roti = 1.5f;

        ctx->v_ctx.venc[i].rc_attr.start_qp = 26;
        ctx->v_ctx.venc[i].rc_attr.step_qp = 8;
        ctx->v_ctx.venc[i].rc_attr.min_qp = 10;
        ctx->v_ctx.venc[i].rc_attr.max_qp = 51;
        ctx->v_ctx.venc[i].rc_attr.min_iqp = 10;
        ctx->v_ctx.venc[i].rc_attr.max_iqp = 51;
        ctx->v_ctx.venc[i].rc_attr.frm_min_qp = 10;
        ctx->v_ctx.venc[i].rc_attr.frm_max_qp = 51;
        ctx->v_ctx.venc[i].rc_attr.frm_min_iqp = 10;
        ctx->v_ctx.venc[i].rc_attr.frm_max_iqp = 51;
        ctx->v_ctx.venc[i].rc_attr.max_re_encode_times = 0;

        ctx->v_ctx.venc[i].full_range_flag = 0;
    }

    ctx->v_ctx.venc_cfg_count = MAX_VENC_CHN;

    /* audio 默认 */
    ctx->a_ctx.ai[0].chn = 0;
    ctx->a_ctx.ai[0].channel = 2;
    ctx->a_ctx.ai[0].samprate = 48000;
    ctx->a_ctx.ai[0].bit = 16;
    ctx->a_ctx.ai_cfg_count = 1;

    for (int i = 0; i < MAX_AENC_CHN; i++) {
        ctx->a_ctx.aenc[i].chn = i;
        ctx->a_ctx.aenc[i].buf_time = 100;
        ctx->a_ctx.aenc[i].period = 1024;
    }
    ctx->a_ctx.aenc_cfg_count = MAX_AENC_CHN;

    return ctx;
}