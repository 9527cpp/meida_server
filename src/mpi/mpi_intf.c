/*
 * MPI 接口实现：单例注册、上下文设置、初始化/反初始化
 */
#include "mpi_intf.h"
#include "../mpi_ctx/mpi_ctx_intf.h"
#include <stdlib.h>
#include <string.h>

static struct mpi_intf *s_mpi_instance;

int mpi_intf_register(struct mpi_intf *mpi)
{
    if (!mpi)
        return -1;
    s_mpi_instance = mpi;
    return 0;
}

struct mpi_intf *mpi_intf_get_instance(void)
{
    return s_mpi_instance;
}

int mpi_intf_init(struct mpi_intf *mpi, struct mpi_ctx *ctx_data)
{
    if (!mpi || !mpi->ctx_data)
        return -1;

    mpi->ctx_data = ctx_data;

    if (mpi->sys && mpi->sys->sys_init) {
        if (mpi->sys->sys_init() != 0) {
            mpi->ctx->free_ctx(ctx_data);
            mpi->ctx_data = NULL;
            return -1;
        }
    }

    if (mpi->video) {
        if (mpi->video->vi_init && mpi->video->vi_init(&ctx_data->v_ctx.vi) != 0)
            goto err_video;
        if (mpi->video->vpss_init && mpi->video->vpss_init(&ctx_data->v_ctx) != 0)
            goto err_vpss;
        for (int i = 0; i < MAX_CHN; i++) {
            if (ctx_data->v_ctx.venc[i].enable) {
                if (mpi->video->venc_init && mpi->video->venc_init(&ctx_data->v_ctx.venc[i]) != 0)
                    goto err_venc;
            }
        }
    }

    if (mpi->audio) {
        if (mpi->audio->ai_init && mpi->audio->ai_init(&ctx_data->a_ctx.ai) != 0)
            goto err_audio;
        for (int i = 0; i < MAX_CHN; i++) {
            if (mpi->audio->aenc_init && mpi->audio->aenc_init(&ctx_data->a_ctx.aenc[i]) != 0)
                goto err_aenc;
        }
    }

    return 0;

err_aenc:
    if (mpi->audio->ai_deinit)
        mpi->audio->ai_deinit(&ctx_data->a_ctx.ai);
err_audio:
err_venc:
    if (mpi->video) {
        for (int j = 0; j < MAX_CHN; j++) {
            if (ctx_data->v_ctx.venc[j].enable) {
                if (mpi->video->venc_deinit)
                    mpi->video->venc_deinit(&ctx_data->v_ctx.venc[j]);
            }
        }
        if (mpi->video->vpss_deinit)
            mpi->video->vpss_deinit(&ctx_data->v_ctx);
        if (mpi->video->vi_deinit)
            mpi->video->vi_deinit(&ctx_data->v_ctx.vi);
    }
    goto do_sys_free;
err_vpss:
    if (mpi->video) {
        if (mpi->video->vpss_deinit)
            mpi->video->vpss_deinit(&ctx_data->v_ctx);
        if (mpi->video->vi_deinit)
            mpi->video->vi_deinit(&ctx_data->v_ctx.vi);
    }
    goto do_sys_free;
err_video:
    if (mpi->video && mpi->video->vi_deinit)
        mpi->video->vi_deinit(&ctx_data->v_ctx.vi);
do_sys_free:
    if (mpi->sys && mpi->sys->sys_deinit)
        mpi->sys->sys_deinit();
    mpi->ctx->free_ctx(ctx_data);
    mpi->ctx_data = NULL;
    return -1;
}

int mpi_intf_deinit(struct mpi_intf *mpi)
{
    if (!mpi || !mpi->ctx_data)
        return 0;

    struct mpi_ctx *ctx_data = mpi->ctx_data;

    if (mpi->audio) {
        for (int i = 0; i < MAX_CHN; i++)
            if (mpi->audio->aenc_deinit)
                mpi->audio->aenc_deinit(&ctx_data->a_ctx.aenc[i]);
        if (mpi->audio->ai_deinit)
            mpi->audio->ai_deinit(&ctx_data->a_ctx.ai);
    }

    if (mpi->video) {
        for (int i = 0; i < MAX_CHN; i++) {
            if (ctx_data->v_ctx.venc[i].enable) {
                if (mpi->video->venc_deinit)
                    mpi->video->venc_deinit(&ctx_data->v_ctx.venc[i]);
            }
        }
        if (mpi->video->vpss_deinit)
            mpi->video->vpss_deinit(&ctx_data->v_ctx);
        if (mpi->video->vi_deinit)
            mpi->video->vi_deinit(&ctx_data->v_ctx.vi);
    }

    if (mpi->sys && mpi->sys->sys_deinit)
        mpi->sys->sys_deinit();

    mpi->ctx->free_ctx(ctx_data);
    mpi->ctx_data = NULL;
    return 0;
}
