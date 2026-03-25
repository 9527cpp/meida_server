#ifndef __MPI_INTF_H__
#define __MPI_INTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 系统操作接口 */
struct mpi_sys_opt {
    int (*sys_init)(void);
    int (*sys_deinit)(void);
};

/* 视频操作接口 */
struct mpi_video_opt {
    int (*vi_init)(void *ctx);
    int (*vi_deinit)(void *ctx);
    int (*vpss_init)(void *ctx);
    int (*vpss_deinit)(void *ctx);
    int (*venc_init)(void *ctx);
    int (*venc_deinit)(void *ctx);
    int (*venc_get_data)(void *ctx, void *data, int *len);
};

/* 音频操作接口 */
struct mpi_audio_opt {
    int (*ai_init)(void *ctx);
    int (*ai_deinit)(void *ctx);
    int (*aenc_init)(void *ctx);
    int (*aenc_deinit)(void *ctx);
    int (*aenc_get_data)(void *ctx, void *data, int *len);
};

/* MPI接口上下文 */
struct mpi_ctx_intf;

/* MPI接口实例 */
struct mpi_intf {
    struct mpi_ctx *ctx_data;   /* 已加载的上下文数据，init 时由 ctx->load() 填充 */
    struct mpi_sys_opt *sys;
    struct mpi_video_opt *video;
    struct mpi_audio_opt *audio;
};

/*
 * 动态库导出约定：每个平台 .so 须导出此符号，返回该平台的 mpi_intf 实例。
 * main 通过 dlsym(handle, MPI_INTF_CREATE_SYMBOL) 获取此函数指针后调用。
 */
#define MPI_INTF_CREATE_SYMBOL "mpi_intf_create"
typedef struct mpi_intf *(*mpi_intf_create_fn)(void);

/* 注册MPI接口实例 */
int mpi_intf_register(struct mpi_intf *mpi);

/* 获取MPI接口实例 */
struct mpi_intf *mpi_intf_get_instance(void);

/* 初始化MPI接口 */
int mpi_intf_init(struct mpi_intf *mpi, struct mpi_ctx *ctx_data);

/* 反初始化MPI接口 */
int mpi_intf_deinit(struct mpi_intf *mpi);

#ifdef __cplusplus
}
#endif

#endif /* __MPI_INTF_H__ */
