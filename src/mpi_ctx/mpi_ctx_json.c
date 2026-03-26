#include "mpi_ctx_intf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "json/cJSON.h"
#define MODULE_TAG "mpi_ctx_json"

#include "log/log_tag.h"


static struct mpi_ctx *json_load_impl(const char *cfg_path)
{
    FILE *fp = fopen(cfg_path, "rb");
    if (!fp) {
        WriteLog(LOG_ERROR, "json file open failed path=%s", cfg_path);
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
        WriteLog(LOG_ERROR, "json parse failed cfg_path=%s err_ptr=%s", cfg_path,
                  cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "(null)");
        return NULL;
    }

    struct mpi_ctx *ctx = (struct mpi_ctx *)calloc(1, sizeof(struct mpi_ctx));
    if (!ctx) {
        cJSON_Delete(root);
        return NULL;
    }

    /* -------------------------
     * vi[]：解析所有 enable=true 的 VI 条目到 vi[] 数组
     * ------------------------- */
    cJSON *vi_arr = cJSON_GetObjectItem(root, "vi");
    int parsed_vi = 0;
    if (vi_arr && vi_arr->type == cJSON_Array) {
        int vi_sz = cJSON_GetArraySize(vi_arr);
        for (int i = 0; i < vi_sz && parsed_vi < MAX_VI_CHN; i++) {
            cJSON *item = cJSON_GetArrayItem(vi_arr, i);
            if (!item || item->type != cJSON_Object)
                continue;

            cJSON *devid = cJSON_GetObjectItem(item, "devid");
            cJSON *pipeid = cJSON_GetObjectItem(item, "pipeid");
            cJSON *chn_index = cJSON_GetObjectItem(item, "chn_index");
            cJSON *entity_name = cJSON_GetObjectItem(item, "entity_name");
            cJSON *format = cJSON_GetObjectItem(item, "format");
            cJSON *width = cJSON_GetObjectItem(item, "width");
            cJSON *height = cJSON_GetObjectItem(item, "height");
            cJSON *buf_cnt = cJSON_GetObjectItem(item, "buf_cnt");
            cJSON *out_file = cJSON_GetObjectItem(item, "out_file");
            cJSON *memory_type = cJSON_GetObjectItem(item, "memory_type");
            cJSON *enable = cJSON_GetObjectItem(item, "enable");
            cJSON *fps = cJSON_GetObjectItem(item, "fps");

            int idx = parsed_vi;

            if (devid && devid->type == cJSON_Number) ctx->v_ctx.vi[idx].devid = devid->valueint;
            if (pipeid && pipeid->type == cJSON_Number) ctx->v_ctx.vi[idx].pipeid = pipeid->valueint;
            if (chn_index && chn_index->type == cJSON_Number) ctx->v_ctx.vi[idx].chn_index = chn_index->valueint;
            if (entity_name && entity_name->type == cJSON_String) ctx->v_ctx.vi[idx].entity_name = strdup(entity_name->valuestring);
            if (format && format->type == cJSON_Number) ctx->v_ctx.vi[idx].format = format->valueint;
            if (width && width->type == cJSON_Number) ctx->v_ctx.vi[idx].width = width->valueint;
            if (height && height->type == cJSON_Number) ctx->v_ctx.vi[idx].height = height->valueint;
            if (buf_cnt && buf_cnt->type == cJSON_Number) ctx->v_ctx.vi[idx].buf_cnt = buf_cnt->valueint;
            if (out_file && out_file->type == cJSON_String) ctx->v_ctx.vi[idx].out_file = strdup(out_file->valuestring);
            if (memory_type && memory_type->type == cJSON_Number) ctx->v_ctx.vi[idx].memory_type = memory_type->valueint;
            if (enable) {
                if (enable->type == cJSON_True) ctx->v_ctx.vi[idx].enable = true;
                else if (enable->type == cJSON_False) ctx->v_ctx.vi[idx].enable = false;
                else if (enable->type == cJSON_Number) ctx->v_ctx.vi[idx].enable = (enable->valueint != 0);
            }
            if (fps && fps->type == cJSON_Number) ctx->v_ctx.vi[idx].fps = fps->valueint;

            WriteLog(LOG_INFO, "vi[%d]: devid=%d pipeid=%d chn_index=%d enable=%d %dx%d fps=%d buf_cnt=%d",
                     idx,
                     ctx->v_ctx.vi[idx].devid, ctx->v_ctx.vi[idx].pipeid, ctx->v_ctx.vi[idx].chn_index, (int)ctx->v_ctx.vi[idx].enable,
                     ctx->v_ctx.vi[idx].width, ctx->v_ctx.vi[idx].height, ctx->v_ctx.vi[idx].fps, ctx->v_ctx.vi[idx].buf_cnt);

            parsed_vi++;
        }
    }
    ctx->v_ctx.vi_cfg_count = parsed_vi;

    /* -------------------------
     * vpss[]：解析所有 enable=true 的 VPSS 条目到 vpss[] 数组
     * ------------------------- */
    cJSON *vpss_arr = cJSON_GetObjectItem(root, "vpss");
    int parsed_vpss = 0;
    if (vpss_arr && vpss_arr->type == cJSON_Array) {
        int vpss_sz = cJSON_GetArraySize(vpss_arr);
        for (int i = 0; i < vpss_sz && parsed_vpss < MAX_VPSS_CHN; i++) {
            cJSON *item = cJSON_GetArrayItem(vpss_arr, i);
            if (!item || item->type != cJSON_Object)
                continue;

            cJSON *chn_idx = cJSON_GetObjectItem(item, "chn_index");
            cJSON *enable = cJSON_GetObjectItem(item, "enable");
            cJSON *width = cJSON_GetObjectItem(item, "width");
            cJSON *height = cJSON_GetObjectItem(item, "height");
            cJSON *out_width = cJSON_GetObjectItem(item, "out_width");
            cJSON *out_height = cJSON_GetObjectItem(item, "out_height");

            int idx = parsed_vpss;

            if (chn_idx && chn_idx->type == cJSON_Number) ctx->v_ctx.vpss[idx].chn_index = chn_idx->valueint;
            if (width && width->type == cJSON_Number) ctx->v_ctx.vpss[idx].width = width->valueint;
            if (height && height->type == cJSON_Number) ctx->v_ctx.vpss[idx].height = height->valueint;
            if (out_width && out_width->type == cJSON_Number) ctx->v_ctx.vpss[idx].out_width = out_width->valueint;
            if (out_height && out_height->type == cJSON_Number) ctx->v_ctx.vpss[idx].out_height = out_height->valueint;
            if (enable) {
                if (enable->type == cJSON_True) ctx->v_ctx.vpss[idx].enable = true;
                else if (enable->type == cJSON_False) ctx->v_ctx.vpss[idx].enable = false;
                else if (enable->type == cJSON_Number) ctx->v_ctx.vpss[idx].enable = (enable->valueint != 0);
            }

            WriteLog(LOG_INFO, "vpss[%d]: chn_index=%d enable=%d %dx%d -> %dx%d",
                     idx,
                     ctx->v_ctx.vpss[idx].chn_index, (int)ctx->v_ctx.vpss[idx].enable,
                     ctx->v_ctx.vpss[idx].width, ctx->v_ctx.vpss[idx].height,
                     ctx->v_ctx.vpss[idx].out_width, ctx->v_ctx.vpss[idx].out_height);

            parsed_vpss++;
        }
    }
    ctx->v_ctx.vpss_cfg_count = parsed_vpss;

    /* -------------------------
     * venc[]：填充 ctx->v_ctx.venc[chn_index]
     * ------------------------- */
    cJSON *venc_arr = cJSON_GetObjectItem(root, "venc");
    int parsed_venc = 0;
    if (venc_arr && venc_arr->type == cJSON_Array) {
        int arr_sz = cJSON_GetArraySize(venc_arr);
        for (int arr_i = 0; arr_i < arr_sz && parsed_venc < MAX_VENC_CHN; arr_i++) {
            cJSON *item = cJSON_GetArrayItem(venc_arr, arr_i);
            if (!item || item->type != cJSON_Object)
                continue;

            cJSON *chn_idx = cJSON_GetObjectItem(item, "chn_index");
            cJSON *enable = cJSON_GetObjectItem(item, "enable");
            cJSON *type = cJSON_GetObjectItem(item, "type");
            cJSON *venc_chn_attr = cJSON_GetObjectItem(item, "venc_chn_attr");
            cJSON *rc_attr = cJSON_GetObjectItem(item, "rc_attr");
            cJSON *video_full_range_flag = cJSON_GetObjectItem(item, "video_full_range_flag");
            cJSON *vui = cJSON_GetObjectItem(item, "vui"); /* 兼容旧配置结构 */

            int chn_index_val = (chn_idx && chn_idx->type == cJSON_Number) ? chn_idx->valueint : arr_i;
            if (chn_index_val < 0 || chn_index_val >= MAX_VENC_CHN)
                continue;

            bool en = false;
            if (enable) {
                if (enable->type == cJSON_True) en = true;
                else if (enable->type == cJSON_False) en = false;
                else if (enable->type == cJSON_Number) en = (enable->valueint != 0);
            }

            if (type && type->type == cJSON_String)
                ctx->v_ctx.venc[chn_index_val].type = strdup(type->valuestring);
            ctx->v_ctx.venc[chn_index_val].chn_index = chn_index_val;
            ctx->v_ctx.venc[chn_index_val].enable = en;

            if (venc_chn_attr && venc_chn_attr->type == cJSON_Object) {
                cJSON *pixel_format = cJSON_GetObjectItem(venc_chn_attr, "pixel_format");
                cJSON *width = cJSON_GetObjectItem(venc_chn_attr, "width");
                cJSON *height = cJSON_GetObjectItem(venc_chn_attr, "height");
                cJSON *vir_width = cJSON_GetObjectItem(venc_chn_attr, "vir_width");
                cJSON *vir_height = cJSON_GetObjectItem(venc_chn_attr, "vir_height");
                cJSON *profile = cJSON_GetObjectItem(venc_chn_attr, "profile");
                cJSON *stream_buf_cnt = cJSON_GetObjectItem(venc_chn_attr, "stream_buf_cnt");

                if (pixel_format && pixel_format->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.pixel_format = pixel_format->valueint;
                if (width && width->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.width = width->valueint;
                if (height && height->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.height = height->valueint;
                if (vir_width && vir_width->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.vir_width = vir_width->valueint;
                if (vir_height && vir_height->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.vir_height = vir_height->valueint;
                if (profile && profile->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.profile = profile->valueint;
                if (stream_buf_cnt && stream_buf_cnt->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].venc_attr.stream_buf_cnt = stream_buf_cnt->valueint;
            }

            if (rc_attr && rc_attr->type == cJSON_Object) {
                cJSON *mode = cJSON_GetObjectItem(rc_attr, "mode");
                if (mode && mode->type == cJSON_String)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.mode = strdup(mode->valuestring);

                cJSON *gop = cJSON_GetObjectItem(rc_attr, "gop");
                cJSON *bitrate = cJSON_GetObjectItem(rc_attr, "bitrate");
                cJSON *src_num = cJSON_GetObjectItem(rc_attr, "src_frame_rate_num");
                cJSON *src_den = cJSON_GetObjectItem(rc_attr, "src_frame_rate_den");
                cJSON *dst_den = cJSON_GetObjectItem(rc_attr, "dst_frame_rate_den");
                cJSON *max_roti = cJSON_GetObjectItem(rc_attr, "max_bitrate_roti");
                cJSON *min_roti = cJSON_GetObjectItem(rc_attr, "min_bitrate_roti");

                if (gop && gop->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].rc_attr.gop = gop->valueint;
                if (bitrate && bitrate->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].rc_attr.bitrate = bitrate->valueint;
                if (src_num && src_num->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].rc_attr.src_frame_rate_num = src_num->valueint;
                if (src_den && src_den->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].rc_attr.src_frame_rate_den = src_den->valueint;
                if (dst_den && dst_den->type == cJSON_Number) ctx->v_ctx.venc[chn_index_val].rc_attr.dst_frame_rate_den = dst_den->valueint;
                if (max_roti && (max_roti->type == cJSON_Number)) ctx->v_ctx.venc[chn_index_val].rc_attr.max_bitrate_roti = (float)max_roti->valuedouble;
                if (min_roti && (min_roti->type == cJSON_Number)) ctx->v_ctx.venc[chn_index_val].rc_attr.min_bitrate_roti = (float)min_roti->valuedouble;

                /* QP 参数：按你的 /tmp/config.json，这些键都在 rc_attr 对象内部 */
                cJSON *first_frame_start_qp = cJSON_GetObjectItem(rc_attr, "first_frame_start_qp");
                cJSON *step_qp = cJSON_GetObjectItem(rc_attr, "step_qp");
                cJSON *delt_ip_qp = cJSON_GetObjectItem(rc_attr, "delt_ip_qp");
                cJSON *min_qp = cJSON_GetObjectItem(rc_attr, "min_qp");
                cJSON *max_qp = cJSON_GetObjectItem(rc_attr, "max_qp");
                cJSON *min_i_qp = cJSON_GetObjectItem(rc_attr, "min_i_qp");
                cJSON *max_i_qp = cJSON_GetObjectItem(rc_attr, "max_i_qp");
                cJSON *frm_min_qp = cJSON_GetObjectItem(rc_attr, "frm_min_qp");
                cJSON *frm_max_qp = cJSON_GetObjectItem(rc_attr, "frm_max_qp");
                cJSON *frm_min_i_qp = cJSON_GetObjectItem(rc_attr, "frm_min_i_qp");
                cJSON *frm_max_i_qp = cJSON_GetObjectItem(rc_attr, "frm_max_i_qp");
                cJSON *reencode_times = cJSON_GetObjectItem(rc_attr, "reencode_times");

                if (first_frame_start_qp && first_frame_start_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.start_qp = first_frame_start_qp->valueint;
                if (step_qp && step_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.step_qp = step_qp->valueint;
                if (delt_ip_qp && delt_ip_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.delt_ip_qp = delt_ip_qp->valueint;
                if (min_qp && min_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.min_qp = min_qp->valueint;
                if (max_qp && max_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.max_qp = max_qp->valueint;
                if (min_i_qp && min_i_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.min_iqp = min_i_qp->valueint;
                if (max_i_qp && max_i_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.max_iqp = max_i_qp->valueint;
                if (frm_min_qp && frm_min_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.frm_min_qp = frm_min_qp->valueint;
                if (frm_max_qp && frm_max_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.frm_max_qp = frm_max_qp->valueint;
                if (frm_min_i_qp && frm_min_i_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.frm_min_iqp = frm_min_i_qp->valueint;
                if (frm_max_i_qp && frm_max_i_qp->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.frm_max_iqp = frm_max_i_qp->valueint;
                if (reencode_times && reencode_times->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].rc_attr.max_re_encode_times = reencode_times->valueint;
            }

            /* full range：你的 /tmp/config.json 是直接放在 venc item 内的 video_full_range_flag */
            if (video_full_range_flag && video_full_range_flag->type == cJSON_Number) {
                ctx->v_ctx.venc[chn_index_val].full_range_flag = video_full_range_flag->valueint;
            } else if (vui && vui->type == cJSON_Object) {
                cJSON *full_range_flag = cJSON_GetObjectItem(vui, "video_full_range_flag");
                if (full_range_flag && full_range_flag->type == cJSON_Number)
                    ctx->v_ctx.venc[chn_index_val].full_range_flag = full_range_flag->valueint;
            }

            if (ctx->v_ctx.venc[chn_index_val].enable)
                parsed_venc++;

            WriteLog(LOG_INFO, "venc[%d]: enable=%d type=%s pf=%d %dx%d stream_buf_cnt=%d rc_mode=%s gop=%d",
                     chn_index_val,
                     (int)ctx->v_ctx.venc[chn_index_val].enable,
                     ctx->v_ctx.venc[chn_index_val].type,
                     ctx->v_ctx.venc[chn_index_val].venc_attr.pixel_format,
                     ctx->v_ctx.venc[chn_index_val].venc_attr.width,
                     ctx->v_ctx.venc[chn_index_val].venc_attr.height,
                     ctx->v_ctx.venc[chn_index_val].venc_attr.stream_buf_cnt,
                     ctx->v_ctx.venc[chn_index_val].rc_attr.mode ? ctx->v_ctx.venc[chn_index_val].rc_attr.mode : "(null)",
                     ctx->v_ctx.venc[chn_index_val].rc_attr.gop);
        }
    }
    ctx->v_ctx.venc_cfg_count = parsed_venc;

    cJSON_Delete(root);
    return ctx;
}

static void json_free_impl(struct mpi_ctx *ctx)
{
    if (!ctx)
        return;

    /* 释放 vi[] 数组中所有条目 */
    for (int i = 0; i < ctx->v_ctx.vi_cfg_count; i++) {
        if (ctx->v_ctx.vi[i].entity_name)
            free(ctx->v_ctx.vi[i].entity_name);
        if (ctx->v_ctx.vi[i].out_file)
            free(ctx->v_ctx.vi[i].out_file);
    }

    /* 释放 venc[] 数组中所有条目 */
    for (int i = 0; i < MAX_VENC_CHN; i++) {
        if (ctx->v_ctx.venc[i].type)
            free(ctx->v_ctx.venc[i].type);
        if (ctx->v_ctx.venc[i].rc_attr.mode)
            free(ctx->v_ctx.venc[i].rc_attr.mode);
    }

    free(ctx);
}

struct mpi_ctx_intf json_ctx_intf = {
    .load = json_load_impl,
    .free_ctx = json_free_impl
};
