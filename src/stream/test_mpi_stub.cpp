/**
 * test_mpi_stub.cpp - MPI stub for stream unit testing
 *
 * Provides minimal MPI symbols needed for linking stream_venc and stream_aenc
 */

#include "mpi/mpi_intf.h"
#include "mpi_ctx/mpi_ctx_intf.h"

// Stub implementations for video
static int stub_vi_init(void *ctx) { (void)ctx; return 0; }
static int stub_vi_deinit(void *ctx) { (void)ctx; return 0; }
static void stub_vi_release_data(void *ctx, void *data, int len) { (void)ctx; (void)data; (void)len; }
static int stub_vpss_init(void *ctx) { (void)ctx; return 0; }
static int stub_vpss_deinit(void *ctx) { (void)ctx; return 0; }
static int stub_vpss_get_data(void *ctx, void *data, int *len) { (void)ctx; (void)data; (void)len; return -1; }
static void stub_vpss_release_data(void *ctx, void *data, int len) { (void)ctx; (void)data; (void)len; }
static int stub_venc_init(void *ctx) { (void)ctx; return 0; }
static int stub_venc_deinit(void *ctx) { (void)ctx; return 0; }
static int stub_venc_get_data(void *ctx, void *data, int *len) {
    (void)ctx; (void)data; (void)len;
    return -1;  // Return -1 (no data) for testing
}
static int stub_venc_release_data(void *ctx, void *data, int len) { (void)ctx; (void)data; (void)len; return 0; }

// Stub implementations for audio
static int stub_ai_init(void *ctx) { (void)ctx; return 0; }
static int stub_ai_deinit(void *ctx) { (void)ctx; return 0; }
static int stub_ai_get_data(void *ctx, void *data, int *len) { (void)ctx; (void)data; (void)len; return -1; }
static void stub_ai_release_data(void *ctx, void *data, int len) { (void)ctx; (void)data; (void)len; }
static int stub_aenc_init(void *ctx) { (void)ctx; return 0; }
static int stub_aenc_deinit(void *ctx) { (void)ctx; return 0; }
static int stub_aenc_get_data(void *ctx, void *data, int *len) { (void)ctx; (void)data; (void)len; return -1; }
static int stub_aenc_release_data(void *ctx, void *data, int len) { (void)ctx; (void)data; (void)len; return 0; }

static int stub_sys_init(void) { return 0; }
static int stub_sys_deinit(void) { return 0; }

static struct mpi_sys_opt stub_sys_opt = {
    .sys_init = stub_sys_init,
    .sys_deinit = stub_sys_deinit,
};

static struct mpi_video_opt stub_video_opt = {
    .vi_init = stub_vi_init,
    .vi_deinit = stub_vi_deinit,
    .vi_release_data = stub_vi_release_data,
    .vpss_init = stub_vpss_init,
    .vpss_deinit = stub_vpss_deinit,
    .vpss_get_data = stub_vpss_get_data,
    .vpss_release_data = stub_vpss_release_data,
    .venc_init = stub_venc_init,
    .venc_deinit = stub_venc_deinit,
    .venc_get_data = stub_venc_get_data,
    .venc_release_data = stub_venc_release_data,
};

static struct mpi_audio_opt stub_audio_opt = {
    .ai_init = stub_ai_init,
    .ai_deinit = stub_ai_deinit,
    .ai_release_data = stub_ai_release_data,
    .aenc_init = stub_aenc_init,
    .aenc_deinit = stub_aenc_deinit,
    .aenc_get_data = stub_aenc_get_data,
    .aenc_release_data = stub_aenc_release_data,
};

// Allocate ctx_data statically for testing
static struct mpi_ctx test_ctx;
static struct mpi_intf test_mpi = {
    .ctx_data = &test_ctx,
    .sys = &stub_sys_opt,
    .video = &stub_video_opt,
    .audio = &stub_audio_opt,
};

static struct mpi_intf *s_mpi_instance = &test_mpi;

struct mpi_intf *mpi_intf_get_instance(void) {
    return s_mpi_instance;
}

int mpi_intf_register(struct mpi_intf *mpi) {
    if (!mpi) return -1;
    s_mpi_instance = mpi;
    return 0;
}

int mpi_intf_init(struct mpi_intf *mpi, struct mpi_ctx *ctx_data) {
    if (!mpi || !ctx_data) return -1;
    mpi->ctx_data = ctx_data;
    return 0;
}

int mpi_intf_deinit(struct mpi_intf *mpi) {
    if (!mpi) return 0;
    mpi->ctx_data = nullptr;
    return 0;
}
