#include "mpi_ctx_intf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "json/cJSON.h"


static struct mpi_ctx *json_load_impl(const char *cfg_path)
{
    FILE *fp = fopen(cfg_path, "rb");
    if (!fp) {
        fprintf(stderr, "[mpi_ctx] json file open failed path=%s\n", cfg_path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long fsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsz <= 0) {
        fclose(fp);
        return NULL;
    }

    char *buf = (char *)malloc((size_t)fsz + 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }

    size_t rd = fread(buf, 1, (size_t)fsz, fp);
    buf[rd] = '\0';
    fclose(fp);

    /* 配置可能包含注释或多余空白；做一次 minify 再解析 */
    cJSON_Minify(buf);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root)
    {
        fprintf(stderr, "[mpi_ctx] json parse failed cfg_path=%s err_ptr=%s\n", cfg_path,
                cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "(null)");
        return NULL;
    }

    struct mpi_ctx *ctx = (struct mpi_ctx *)calloc(1, sizeof(struct mpi_ctx));
    if (!ctx) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON *venc_arr = cJSON_GetObjectItem(root, "venc");
    if (!venc_arr || venc_arr->type != cJSON_Array) {
        cJSON_Delete(root);
        return ctx;
    }

    int idx = 0;
    int arr_sz = cJSON_GetArraySize(venc_arr);
    for (int arr_i = 0; arr_i < arr_sz && idx < MAX_CHN; arr_i++) {
        cJSON *item = cJSON_GetArrayItem(venc_arr, arr_i);
        if (!item)
            continue;

        cJSON *chn_idx = cJSON_GetObjectItem(item, "chn_index");
        cJSON *enable = cJSON_GetObjectItem(item, "enable");
        cJSON *venc_attr = cJSON_GetObjectItem(item, "venc_chn_attr");
        cJSON *entype = NULL;
        cJSON *stream_buf_cnt = NULL;

        int chn_index_val = (chn_idx && chn_idx->type == cJSON_Number) ? chn_idx->valueint : idx;
        int enable_val = 0;
        if (enable) {
            if (enable->type == cJSON_True)
                enable_val = 1;
            else if (enable->type == cJSON_False)
                enable_val = 0;
            else if (enable->type == cJSON_Number)
                enable_val = enable->valueint ? 1 : 0;
        }

        ctx->v_ctx.venc[idx].chn = chn_index_val;
        ctx->v_ctx.venc[idx].enable = enable_val;

        if (venc_attr && venc_attr->type == cJSON_Object) {
            entype = cJSON_GetObjectItem(venc_attr, "entype");
            stream_buf_cnt = cJSON_GetObjectItem(venc_attr, "stream_buf_cnt");

            if (entype && entype->type == cJSON_Number)
                ctx->v_ctx.venc[idx].entype = entype->valueint;
            if (stream_buf_cnt && stream_buf_cnt->type == cJSON_Number)
                ctx->v_ctx.venc[idx].buf_cnt = stream_buf_cnt->valueint;
        }

        /* 其它字段暂用 NULL 的值；后续可按你需要继续解析 rc_attr 等 */
        idx++;
    }

    ctx->v_ctx.venc_cfg_count = idx;
    /* idx 之后的逻辑通道都标记为 disable，确保不会被 mpi_intf_init 初始化 */
    for (int i = idx; i < MAX_CHN; i++)
        ctx->v_ctx.venc[i].enable = 0;

    cJSON_Delete(root);
    return ctx;
}

static void json_free_impl(struct mpi_ctx *ctx)
{
    free(ctx);
}

struct mpi_ctx_intf json_ctx_intf = {
    .load = json_load_impl,
    .free_ctx = json_free_impl
};