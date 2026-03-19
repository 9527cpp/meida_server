/*
 * MPI 上下文配置接口实现：json_cfg / uci_cfg
 */
#include "mpi_ctx_intf.h"
#include <stdlib.h>
#include <string.h>

static struct mpi_ctx *default_ctx(void)
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
        ctx->v_ctx.venc[i].bitrate = 2000000;
        ctx->v_ctx.venc[i].frame_rate = 25;
        ctx->v_ctx.venc[i].entype = 0; /* e.g. H264 */
        ctx->v_ctx.venc[i].buf_cnt = 3;
    }
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

static struct mpi_ctx *json_load_impl(void);
static void json_free_impl(struct mpi_ctx *ctx);

/* JSON 配置：可从文件路径加载，此处返回默认配置，后续可接 json 解析 */
static const char *s_json_cfg_path;

struct mpi_ctx_intf *mpi_ctx_intf_json_create(const char *cfg_path)
{
    static struct mpi_ctx_intf intf;
    s_json_cfg_path = cfg_path ? cfg_path : "";
    intf.load = json_load_impl;
    intf.free_ctx = json_free_impl;
    return &intf;
}

static struct mpi_ctx *json_load_impl(void)
{
    /* TODO: 从 s_json_cfg_path 读取 JSON 并解析填充 mpi_ctx */
    (void)s_json_cfg_path;
    return default_ctx();
}

static void json_free_impl(struct mpi_ctx *ctx)
{
    free(ctx);
}

/* UCI 配置：从 UCI 读取，此处返回默认配置 */
static struct mpi_ctx *uci_load_impl(void);
static void uci_free_impl(struct mpi_ctx *ctx);

static struct mpi_ctx_intf uci_intf_storage;

struct mpi_ctx_intf *mpi_ctx_intf_uci_create(void)
{
    uci_intf_storage.load = uci_load_impl;
    uci_intf_storage.free_ctx = uci_free_impl;
    return &uci_intf_storage;
}

static struct mpi_ctx *uci_load_impl(void)
{
    /* TODO: 从 UCI 读取配置并填充 mpi_ctx */
    return default_ctx();
}

static void uci_free_impl(struct mpi_ctx *ctx)
{
    free(ctx);
}
