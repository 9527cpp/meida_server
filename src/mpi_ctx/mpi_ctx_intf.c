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
    ctx->s_ctx.devid = 0;
    ctx->s_ctx.pipeid = 0;
    ctx->v_ctx.vi.chn = 0;
    ctx->v_ctx.vi.buf_cnt = 3;
    for (int i = 0; i < MAX_CHN; i++) {
        ctx->v_ctx.venc[i].chn = i;
        ctx->v_ctx.venc[i].enable = 1;
        ctx->v_ctx.venc[i].bitrate = 2000000;
        ctx->v_ctx.venc[i].frame_rate = 25;
        ctx->v_ctx.venc[i].entype = 0; /* e.g. H264 */
        ctx->v_ctx.venc[i].buf_cnt = 3;
    }
    ctx->v_ctx.venc_cfg_count = MAX_CHN;
    ctx->a_ctx.ai.chn = 0;
    ctx->a_ctx.ai.channel = 2;
    ctx->a_ctx.ai.samprate = 48000;
    ctx->a_ctx.ai.bit = 16;
    for (int i = 0; i < MAX_CHN; i++) {
        ctx->a_ctx.aenc[i].chn = i;
        ctx->a_ctx.aenc[i].buf_time = 100;
        ctx->a_ctx.aenc[i].period = 1024;
    }
    return ctx;
}