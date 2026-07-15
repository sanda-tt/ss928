/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: sample_svp_npu_process.c
 * Author: Hisilicon multimedia software group
 * Create: 2024-12-09
 */
#include "sample_svp_npu_process.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <linux/limits.h>
#include "svp_acl_rt.h"
#include "svp_acl.h"
#include "svp_acl_ext.h"

#define CONFIG_V110 0
#define SAMPLE_SVP_NPU_SHAERD_WORK_BUF_NUM     1

#define SVP_NPU_VO_WIDTH    1920
#define SVP_NPU_VO_HEIGHT   1080
#define SAMPLE_SVP_NPU_OVD_PREALLOC_MMZ 430080 /* if not enough: change it within mmz malloc limit */
#define SAMPLE_SVP_NPU_VGS_RGN_CNT 30
#define SAMPLE_SVP_NPU_OVD_INPUT_FILE_NUM 2

static td_u32 lineCount = 0;
static td_u32 g_svp_npu_mmz_alloc_base_offset = 0;
static td_void *g_svp_npu_mmz_alloc_base_attr = TD_NULL;
static ot_vgs_osd *g_osd_buf = TD_NULL;

static td_float g_nms_threshold = 0.7;
static td_float g_low_score_threshold = 0.3;

static td_u32 inputClassId[CLASS_NUM] = {0};
static td_u32 g_top_box_num = {0};
static td_float g_tmp_insert_sort_buf[7 * 8400] = {0.0};
static td_float g_valid_box[7 * 8400] = {0.0};
static td_float g_save_box[7 * 8400] = {0.0};
static td_float g_nms_box[7 * 8400] = {0.0};
static td_float g_tmp_box[8 * 8400] = {0.0};
static td_float g_res_box[6 * 8400] = {0.0};

static ot_vb_pool_info g_svp_npu_vb_pool_info;
static td_void *g_svp_npu_vb_virt_addr = TD_NULL;
static td_bool g_svp_npu_terminate_signal = TD_FALSE;
static sample_svp_npu_shared_work_buf g_svp_npu_shared_work_buf[SAMPLE_SVP_NPU_SHAERD_WORK_BUF_NUM] = {0};
static pthread_t g_svp_npu_thread = 0;
static sample_svp_npu_task_info g_svp_npu_task[SAMPLE_SVP_NPU_MAX_TASK_NUM] = {0};

static td_s32 g_svp_npu_dev_id = 0;
static sample_vi_cfg g_vi_config;
static ot_sample_svp_rect_info g_svp_npu_rect_info = {0};
static td_bool g_svp_npu_thread_stop = TD_FALSE;
static pthread_t g_svp_npu_vdec_thread;

static td_bool g_is_pc_sample = TD_FALSE;
static td_u32 g_input_height = 0;
static td_u32 g_input_width = 0;

static td_u32 g_top_left_x = 0;
static td_u32 g_top_left_y = 1;
static td_u32 g_bottom_right_x = 2;
static td_u32 g_bottom_right_y = 3;
static td_u32 g_score = 4;
static td_u32 g_class_id = 5;
static td_u32 g_bbox_size = 6;
static td_u32 g_bbox_new_size = 7;

#if CONFIG_V110
static ot_sample_svp_media_cfg g_svp_npu_media_cfg = {
    .svp_switch = {TD_FALSE, TD_TRUE},
    .pic_type = {PIC_1080P, PIC_CIF},
    .chn_num = OT_SVP_MAX_VPSS_CHN_NUM,
};
static sample_vo_cfg g_svp_npu_vo_cfg = {
    .vo_dev            = SAMPLE_VO_DEV_UHD,
    .vo_layer          = SAMPLE_VO_LAYER_VHD0,
    .vo_intf_type      = OT_VO_INTF_BT1120,
    .intf_sync         = OT_VO_OUT_1080P60,
    .bg_color          = COLOR_RGB_BLACK,
    .pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect         = {0, 0, SVP_NPU_VO_WIDTH, SVP_NPU_VO_HEIGHT},
    .image_size        = {SVP_NPU_VO_WIDTH, SVP_NPU_VO_HEIGHT},
    .vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len       = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode           = VO_MODE_1MUX,
    .compress_mode     = OT_COMPRESS_MODE_NONE,
};
#else
static ot_sample_svp_media_cfg g_svp_npu_media_cfg = {
    .svp_switch = {TD_FALSE, TD_TRUE},
    .pic_type = {PIC_3840X2160, PIC_CIF},
    .chn_num = OT_SVP_MAX_VPSS_CHN_NUM,
};
static sample_vo_cfg g_svp_npu_vo_cfg = {0};
#endif

static sample_vdec_attr g_svp_npu_vdec_cfg = {
    .type = OT_PT_H264,
    .mode = OT_VDEC_SEND_MODE_FRAME,
    .width = _4K_WIDTH,
    .height = _4K_HEIGHT,
    .sample_vdec_video.dec_mode = OT_VIDEO_DEC_MODE_IP,
    .sample_vdec_video.bit_width = OT_DATA_BIT_WIDTH_8,
    .sample_vdec_video.ref_frame_num = 2, /* 2:ref_frame_num */
    .display_frame_num = 2,               /* 2:display_frame_num */
    .frame_buf_cnt = 5,                   /* 5:2+2+1 */
};

static vdec_thread_param g_svp_npu_vdec_param = {
    .chn_id = 0,
    .type = OT_PT_H264,
    .stream_mode = OT_VDEC_SEND_MODE_FRAME,
    .interval_time = 1000, /* 1000:interval_time */
    .pts_init = 0,
    .pts_increase = 0,
    .e_thread_ctrl = THREAD_CTRL_START,
    .circle_send = TD_TRUE,
    .milli_sec = 0,
    .min_buf_size = (_4K_WIDTH * _4K_HEIGHT * 3) >> 1, /* 3:chn_size */
    .c_file_path = "./data/",
    .c_file_name = "dolls_video.h264",
    .fps = 30,
};

ot_sample_svp_media_cfg *sample_svp_npu_get_media_cfg(td_void)
{
    return &g_svp_npu_media_cfg;
}

td_s32 *sample_svp_npu_get_device_id(td_void)
{
    return &g_svp_npu_dev_id;
}

ot_sample_svp_rect_info *sample_svp_npu_get_svp_rect_info(td_void)
{
    return &g_svp_npu_rect_info;
}

td_bool *sample_svp_npu_get_thread_stop(td_void)
{
    return &g_svp_npu_thread_stop;
}

sample_svp_npu_task_info *sample_svp_npu_get_task_info(int task_idx)
{
    return &g_svp_npu_task[task_idx];
}

sample_vdec_attr *sample_svp_npu_get_vdec_cfg(td_void)
{
    return &g_svp_npu_vdec_cfg;
}

vdec_thread_param *sample_svp_npu_get_vdec_param(td_void)
{
    return &g_svp_npu_vdec_param;
}

static td_void sample_svp_npu_acl_terminate(td_void)
{
    if (g_svp_npu_terminate_signal == TD_TRUE) {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
}

/* function : svp npu signal handle */
td_void sample_svp_npu_acl_handle_sig(td_void)
{
    g_svp_npu_terminate_signal = TD_TRUE;
}

static td_void sample_svp_npu_acl_deinit(td_void)
{
    svp_acl_error ret;

    ret = svp_acl_rt_reset_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("reset device fail\n");
    }
    sample_svp_trace_info("end to reset device is %d\n", g_svp_npu_dev_id);

    ret = svp_acl_finalize();
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("finalize acl fail\n");
    }
    sample_svp_trace_info("end to finalize acl\n");
    (td_void)sample_common_svp_check_sys_exit();
}

#if CONFIG_V110
static td_s32 sample_svp_npu_acl_init(const td_char *acl_config_path)
{
    /* svp acl init */
    svp_acl_rt_run_mode run_mode;
    svp_acl_error ret;
    td_bool is_mpi_init;

    is_mpi_init = sample_common_svp_check_sys_init();
    sample_svp_check_exps_return(is_mpi_init != TD_TRUE, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "mpi init failed!\n");

    ret = svp_acl_init(acl_config_path);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "acl init failed!\n");

    sample_svp_trace_info("svp acl init success!\n");

    /* open device */
    ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        (td_void)svp_acl_finalize();
        sample_svp_trace_err("svp acl open device %d failed!\n", g_svp_npu_dev_id);
        return TD_FAILURE;
    }
    sample_svp_trace_info("open device %d success!\n", g_svp_npu_dev_id);

    /* get run mode */
    ret = svp_acl_rt_get_run_mode(&run_mode);
    if ((ret != SVP_ACL_SUCCESS) || (run_mode != SVP_ACL_DEVICE)) {
        (td_void)svp_acl_rt_reset_device(g_svp_npu_dev_id);
        (td_void)svp_acl_finalize();
        sample_svp_trace_err("acl get run mode failed!\n");
        return TD_FAILURE;
    }
    sample_svp_trace_info("get run mode success!\n");

    return TD_SUCCESS;
}
#else
static td_s32 sample_svp_npu_acl_init(const td_char *acl_config_path, td_bool vi_en)
{
    /* svp acl init */
    svp_acl_rt_run_mode run_mode;
    svp_acl_error ret;
    td_bool is_mpi_init;

    is_mpi_init = sample_common_svp_check_sys_init(vi_en);
    sample_svp_check_exps_return(is_mpi_init != TD_TRUE, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "mpi init failed!\n");

    ret = svp_acl_init(acl_config_path);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "acl init failed!\n");

    sample_svp_trace_info("svp acl init success!\n");

    /* open device */
    ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        (td_void)svp_acl_finalize();
        sample_svp_trace_err("svp acl open device %d failed!\n", g_svp_npu_dev_id);
        return TD_FAILURE;
    }
    sample_svp_trace_info("open device %d success!\n", g_svp_npu_dev_id);

    /* get run mode */
    ret = svp_acl_rt_get_run_mode(&run_mode);
    if ((ret != SVP_ACL_SUCCESS) || (run_mode != SVP_ACL_DEVICE)) {
        (td_void)svp_acl_rt_reset_device(g_svp_npu_dev_id);
        (td_void)svp_acl_finalize();
        sample_svp_trace_err("acl get run mode failed!\n");
        return TD_FAILURE;
    }
    sample_svp_trace_info("get run mode success!\n");

    return TD_SUCCESS;
}
#endif

static td_s32 sample_svp_npu_acl_dataset_init(td_u32 task_idx)
{
    td_s32 ret = sample_common_svp_npu_create_input(&g_svp_npu_task[task_idx]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "create input failed!\n");

    ret = sample_common_svp_npu_create_output(&g_svp_npu_task[task_idx]);
    if (ret != TD_SUCCESS) {
        sample_common_svp_npu_destroy_input(&g_svp_npu_task[task_idx]);
        sample_svp_trace_err("execute create output fail.\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void sample_svp_npu_acl_dataset_deinit(td_u32 task_idx)
{
    (td_void)sample_common_svp_npu_destroy_input(&g_svp_npu_task[task_idx]);
    (td_void)sample_common_svp_npu_destroy_output(&g_svp_npu_task[task_idx]);
}

static td_void sample_svp_npu_acl_deinit_task(td_u32 task_num, td_u32 shared_work_buf_idx)
{
    td_u32 task_idx;

    for (task_idx = 0; task_idx < task_num; task_idx++) {
        (td_void)sample_common_svp_npu_destroy_work_buf(&g_svp_npu_task[task_idx]);
        (td_void)sample_common_svp_npu_destroy_task_buf(&g_svp_npu_task[task_idx]);
        (td_void)sample_svp_npu_acl_dataset_deinit(task_idx);
        (td_void)memset_s(&g_svp_npu_task[task_idx], sizeof(sample_svp_npu_task_cfg), 0,
            sizeof(sample_svp_npu_task_cfg));
    }
    if (g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_ptr != TD_NULL) {
        (td_void)svp_acl_rt_free(g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_ptr);
        g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_ptr = TD_NULL;
        g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_size = 0;
        g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_stride = 0;
    }
}

static td_s32 sample_svp_npu_acl_create_shared_work_buf(td_u32 task_num, td_u32 shared_work_buf_idx)
{
    td_u32 task_idx, work_buf_size, work_buf_stride;
    td_s32 ret;

    for (task_idx = 0; task_idx < task_num; task_idx++) {
        ret = sample_common_svp_npu_get_work_buf_info(&g_svp_npu_task[task_idx], &work_buf_size, &work_buf_stride);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th task work buf info failed!\n", task_idx);

        if (g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_size < work_buf_size) {
            g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_size = work_buf_size;
            g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_stride = work_buf_stride;
        }
    }
    ret = svp_acl_rt_malloc_cached(&g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_ptr,
        g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_size, SVP_ACL_MEM_MALLOC_NORMAL_ONLY);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "malloc %u-th shared work buf failed!\n", shared_work_buf_idx);

    (td_void)svp_acl_rt_mem_flush(g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_ptr,
        g_svp_npu_shared_work_buf[shared_work_buf_idx].work_buf_size);
    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_acl_init_task(td_u32 task_num, td_bool is_share_work_buf,
    td_u32 shared_work_buf_idx)
{
    td_u32 task_idx;
    td_s32 ret;

    if (is_share_work_buf == TD_TRUE) {
        ret = sample_svp_npu_acl_create_shared_work_buf(task_num, shared_work_buf_idx);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "create shared work buf failed!\n");
    }

    for (task_idx = 0; task_idx < task_num; task_idx++) {
        ret = sample_svp_npu_acl_dataset_init(task_idx);
        if (ret != TD_SUCCESS) {
            goto task_init_end_0;
        }
        ret = sample_common_svp_npu_create_task_buf(&g_svp_npu_task[task_idx]);
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("create task buf failed.\n");
            goto task_init_end_0;
        }
        if (is_share_work_buf == TD_FALSE) {
            ret = sample_common_svp_npu_create_work_buf(&g_svp_npu_task[task_idx]);
        } else {
            /* if all tasks are on the same stream, work buf can be shared */
            ret = sample_common_svp_npu_share_work_buf(&g_svp_npu_shared_work_buf[shared_work_buf_idx],
                &g_svp_npu_task[task_idx]);
        }
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("create work buf failed.\n");
            goto task_init_end_0;
        }
    }
    return TD_SUCCESS;

task_init_end_0:
    (td_void)sample_svp_npu_acl_deinit_task(task_num, shared_work_buf_idx);
    return ret;
}

static td_s32 sample_svp_npu_acl_vb_map(td_u32 vb_pool_idx)
{
    td_s32 ret;

    if (g_svp_npu_vb_virt_addr == TD_NULL) {
        ret = ss_mpi_vb_get_pool_info(g_svp_npu_media_cfg.vb_pool[vb_pool_idx], &g_svp_npu_vb_pool_info);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get pool info failed!\n");
        g_svp_npu_vb_virt_addr = ss_mpi_sys_mmap(g_svp_npu_vb_pool_info.pool_phy_addr,
            g_svp_npu_vb_pool_info.pool_size);
        sample_svp_check_exps_return(g_svp_npu_vb_virt_addr == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "map vb pool failed!\n");
    }
    return TD_SUCCESS;
}

static td_void sample_svp_npu_acl_pause(td_void)
{
    printf("---------------press Enter key to exit!---------------\n");
    if (g_svp_npu_terminate_signal == TD_TRUE) {
        return;
    }
    (td_void)getchar();
    if (g_svp_npu_terminate_signal == TD_TRUE) {
        return;
    }
}

static td_void sample_svp_npu_acl_set_task_info(td_u32 task_idx, td_u32 model_idx)
{
    g_svp_npu_task[task_idx].cfg.max_batch_num = 1;
    g_svp_npu_task[task_idx].cfg.dynamic_batch_num = 1;
    g_svp_npu_task[task_idx].cfg.total_t = 0;
    g_svp_npu_task[task_idx].cfg.is_cached = TD_TRUE;
    g_svp_npu_task[task_idx].cfg.model_idx = model_idx;
}

static td_s32 sample_svp_npu_sort_by_box_score(td_float *valid_box, td_u32 *valid_box_num, td_u32 offset,
    td_u32 confbit)
{
    /* InsertSort */
    td_s32 flag = 1;
    (td_void)memset_s(g_tmp_insert_sort_buf, sizeof(g_tmp_insert_sort_buf), 0, sizeof(g_tmp_insert_sort_buf));
    for (td_u32 i = 0; i < *valid_box_num && flag; i++) {
        td_float key = valid_box[i * offset + confbit];
        for (td_u32 k = 0; k < offset; k++) {
            g_tmp_insert_sort_buf[k] = valid_box[i * offset + k];
        }
        td_s32 j = (td_s32)i - 1;
        while (j >= 0 && valid_box[(td_u32)j * offset + confbit] < key) {
            for (td_u32 k = 0; k < offset; k++) {
                valid_box[(td_u32)j * offset + k + offset] =  valid_box[(td_u32)j * offset + k];
            }
            j--;
        }
        for (td_u32 k = 0; k < offset; k++) {
            valid_box[(td_u32)j * offset + k + offset] =  g_tmp_insert_sort_buf[k];
        }
    }
    return TD_SUCCESS;
}

static td_float sample_svp_ovd_calc_iou(td_float *box1, size_t len1, td_float *box2, size_t len2, td_u32 j)
{
    if (len1 <= 0 || len2 <= 0) {
        sample_svp_trace_err("box1 len is 0, or box2 len is 0, sample_svp_ovd_calc_iou failed!\n");
        return 0;
    }

    td_float area1 = box1[g_bbox_size];
    td_float area2 = box2[j * (g_bbox_new_size + 1) + g_bbox_size];
    td_float xx1 = (box1[g_top_left_x] > box2[j * (g_bbox_new_size + 1) + g_top_left_x] ?
        box1[g_top_left_x] : box2[j * (g_bbox_new_size + 1) + g_top_left_x]);
    td_float yy1 = (box1[g_top_left_y] > box2[j * (g_bbox_new_size + 1) + g_top_left_y] ?
        box1[g_top_left_y] : box2[j * (g_bbox_new_size + 1) + g_top_left_y]);
    td_float xx2 = (box1[g_bottom_right_x] < box2[j * (g_bbox_new_size + 1) + g_bottom_right_x] ?
        box1[g_bottom_right_x] : box2[j * (g_bbox_new_size + 1) + g_bottom_right_x]);
    td_float yy2 = (box1[g_bottom_right_y] < box2[j * (g_bbox_new_size + 1) + g_bottom_right_y] ?
        box1[g_bottom_right_y] : box2[j * (g_bbox_new_size + 1) + g_bottom_right_y]);
    td_float w = ((xx2 - xx1) > 0.0 ? (xx2 - xx1) : 0.0);
    td_float h = ((yy2 - yy1) > 0.0 ? (yy2 - yy1) : 0.0);
    td_float inter = w * h;
    td_float ovr = inter / (area1 + area2 - inter);
    return ovr;
}

static td_s32 sample_gen_ovd_cpu_postprocess_write_result(td_u32 tmp_box_num)
{
    sample_svp_check_exps_return(tmp_box_num == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "ovd cpu postprocess output box num is zero!\n");

    FILE *fp = TD_NULL;
    td_char path[PATH_MAX + 1] = { 0 };
    td_s32 ret = 0;

    const td_char *box_result_file = "./result/txtResult/box_result.txt";
    sample_svp_check_exps_return(realpath(box_result_file, path) == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error read_file, Invalid %s file or path!\n", box_result_file);

    fp = fopen(path, "w");
    sample_svp_check_exps_return(fp == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "open file failed!\n");
    ret = fprintf(fp, "%u %u\n", g_input_width, g_input_height);
    sample_svp_check_exps_goto(ret < 0, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Read file failed!\n");

    for (td_u32 i = 0; i < tmp_box_num; ++i) {
        ret = fprintf(fp, "%u  %f  %f  %f  %f  %f \n",
            (td_u32)g_res_box[i * g_bbox_size + g_class_id], g_res_box[i * g_bbox_size + g_score],
            g_res_box[i * g_bbox_size + g_top_left_x], g_res_box[i * g_bbox_size + g_top_left_y],
            g_res_box[i * g_bbox_size + g_bottom_right_x], g_res_box[i * g_bbox_size + g_bottom_right_y]);
        sample_svp_check_exps_goto(ret < 0, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Read file failed!\n");
    }

    (td_void)fclose(fp);
    return TD_SUCCESS;

end:
    if (fp != TD_NULL) {
        (td_void)fclose(fp);
    }
    return TD_FAILURE;
}

static td_s32 sample_gen_ovd_cpu_postprocess_output_result_box(td_u32 tmp_box_num)
{
    g_top_box_num = 0;
    (td_void)memset_s(g_res_box, sizeof(g_res_box), 0, sizeof(g_res_box));
    for (td_u32 i = 0; i < tmp_box_num; i++) {
        if (g_top_box_num >= TOP_K_LIMIT) { // select top300 boxes to output
            break;
        }
        g_res_box[i * g_bbox_size + g_top_left_x] =
            g_save_box[(td_u32)g_tmp_box[i * (g_bbox_new_size + 1) + g_bbox_new_size] * g_bbox_new_size + g_top_left_x];
        g_res_box[i * g_bbox_size + g_top_left_y] = g_save_box[(td_u32)g_tmp_box[i *
            (g_bbox_new_size + 1) + g_bbox_new_size] * g_bbox_new_size + g_top_left_y];
        g_res_box[i * g_bbox_size + g_bottom_right_x] = g_save_box[(td_u32)g_tmp_box[i *
            (g_bbox_new_size + 1) + g_bbox_new_size] * g_bbox_new_size + g_bottom_right_x];
        g_res_box[i * g_bbox_size + g_bottom_right_y] = g_save_box[(td_u32)g_tmp_box[i *
            (g_bbox_new_size + 1) + g_bbox_new_size] * g_bbox_new_size + g_bottom_right_y];
        g_res_box[i * g_bbox_size + g_score] =
            g_save_box[(td_u32)g_tmp_box[i * (g_bbox_new_size + 1) + g_bbox_new_size] * g_bbox_new_size + g_score];
        g_res_box[i * g_bbox_size + g_class_id] =
            g_save_box[(td_u32)g_tmp_box[i * (g_bbox_new_size + 1) + g_bbox_new_size] * g_bbox_new_size + g_class_id];
        g_top_box_num++;
    }
    td_s32 ret = sample_svp_npu_sort_by_box_score(g_res_box, &g_top_box_num, g_bbox_size, g_class_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "sample_svp_npu_sort_by_box_score failed!\n");
    printf("\ncurrent class valid box number is: %d\n", g_top_box_num);
    for (td_u32 i = 0; i < g_top_box_num; i++) {
        printf("    [lx: %lf, ly: %lf, rx: %lf, ry: %lf, score: %lf; class id: %d]\n",
            g_res_box[i * g_bbox_size + g_top_left_x], g_res_box[i * g_bbox_size + g_top_left_y],
            g_res_box[i * g_bbox_size + g_bottom_right_x], g_res_box[i * g_bbox_size + g_bottom_right_y],
            g_res_box[i * g_bbox_size + g_score], (td_s32)g_res_box[i * g_bbox_size + g_class_id]);
    }
    if (g_is_pc_sample) {
        ret = sample_gen_ovd_cpu_postprocess_write_result(g_top_box_num);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "sample_gen_ovd_cpu_postprocess_write_result failed!\n");
    }
    return ret;
}

static td_s32 sample_gen_ovd_cpu_postprocess_multiclass_nms(td_float *valid_box, td_u32 valid_box_num)
{
    td_u32 tmp_box_num = 0;
    (td_void)memset_s(g_tmp_box, sizeof(g_tmp_box), 0, sizeof(g_tmp_box));
    for (td_u32 i = 0; i < valid_box_num; i++) {
        td_float area = (valid_box[i * g_bbox_new_size + g_bottom_right_x] -
            valid_box[i * g_bbox_new_size + g_top_left_x]) * (valid_box[i * g_bbox_new_size + g_bottom_right_y] -
            valid_box[i * g_bbox_new_size + g_top_left_y]);

        td_float bbox[8] = { // 8: size
            valid_box[i * g_bbox_new_size + g_top_left_x],
            valid_box[i * g_bbox_new_size + g_top_left_y],
            valid_box[i * g_bbox_new_size + g_bottom_right_x],
            valid_box[i * g_bbox_new_size + g_bottom_right_y],
            valid_box[i * g_bbox_new_size + g_score],
            valid_box[i * g_bbox_new_size + g_class_id],
            area,
            valid_box[i * g_bbox_new_size + g_bbox_size]};
        td_bool keep = 1;
        for (td_u32 j = 0; j < tmp_box_num; j++) {
            if (bbox[g_class_id] == g_tmp_box[j * (g_bbox_new_size + 1) + g_class_id] &&
                sample_svp_ovd_calc_iou(bbox, sizeof(bbox), g_tmp_box, sizeof(g_tmp_box), j) > g_nms_threshold) {
                keep = 0;
                break;
            }
        }
        if (keep) {
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_top_left_x] = bbox[g_top_left_x];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_top_left_y] = bbox[g_top_left_y];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_bottom_right_x] = bbox[g_bottom_right_x];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_bottom_right_y] = bbox[g_bottom_right_y];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_score] = bbox[g_score];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_class_id] = bbox[g_class_id];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + g_bbox_size] = bbox[g_bbox_size];
            g_tmp_box[tmp_box_num * (g_bbox_new_size + 1) + (g_bbox_size + 1)] = bbox[g_bbox_size + 1];
            tmp_box_num++;
        }
    }

    if (tmp_box_num == 0) {
        sample_svp_trace_warning("total valid num is zero\n");
    }

    td_s32 ret = sample_gen_ovd_cpu_postprocess_output_result_box(tmp_box_num);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "sample_gen_ovd_cpu_postprocess_output_result_box failed!\n");
    return ret;
}

static td_s32 sample_gen_ovd_cpu_postprocess_select_topk_box_into_nms(td_u32 valid_box_num)
{
    td_u32 nmsBoxNum = 0;
    (td_void)memset_s(g_nms_box, sizeof(g_nms_box), 0, sizeof(g_nms_box));
    if (valid_box_num > NMS_BOX_THR_OPT) {
        for (td_u32 k = 0; k < valid_box_num; k++) {
            if (nmsBoxNum >= NMS_BOX_MAX_NUM) { // top 30000 boxes into nms
                break;
            }
            g_nms_box[k * g_bbox_new_size + g_top_left_x] = g_valid_box[k * g_bbox_new_size + g_top_left_x];
            g_nms_box[k * g_bbox_new_size + g_top_left_y] = g_valid_box[k * g_bbox_new_size + g_top_left_y];
            g_nms_box[k * g_bbox_new_size + g_bottom_right_x] = g_valid_box[k * g_bbox_new_size + g_bottom_right_x];
            g_nms_box[k * g_bbox_new_size + g_bottom_right_y] = g_valid_box[k * g_bbox_new_size + g_bottom_right_y];
            g_nms_box[k * g_bbox_new_size + g_score] = g_valid_box[k * g_bbox_new_size + g_score];
            g_nms_box[k * g_bbox_new_size + g_class_id] = g_valid_box[k * g_bbox_new_size + g_class_id];
            g_nms_box[k * g_bbox_new_size + g_bbox_size] = g_valid_box[k * g_bbox_new_size + g_bbox_size];
            nmsBoxNum++;
        }
    } else {
        nmsBoxNum = valid_box_num;
        for (td_u32 k = 0; k < nmsBoxNum; k++) {
            g_nms_box[k * g_bbox_new_size + g_top_left_x] = g_valid_box[k * g_bbox_new_size + g_top_left_x];
            g_nms_box[k * g_bbox_new_size + g_top_left_y] = g_valid_box[k * g_bbox_new_size + g_top_left_y];
            g_nms_box[k * g_bbox_new_size + g_bottom_right_x] = g_valid_box[k * g_bbox_new_size + g_bottom_right_x];
            g_nms_box[k * g_bbox_new_size + g_bottom_right_y] = g_valid_box[k * g_bbox_new_size + g_bottom_right_y];
            g_nms_box[k * g_bbox_new_size + g_score] = g_valid_box[k * g_bbox_new_size + g_score];
            g_nms_box[k * g_bbox_new_size + g_class_id] = g_valid_box[k * g_bbox_new_size + g_class_id];
            g_nms_box[k * g_bbox_new_size + g_bbox_size] = g_valid_box[k * g_bbox_new_size + g_bbox_size];
        }
    }
    td_s32 ret = sample_gen_ovd_cpu_postprocess_multiclass_nms(g_nms_box, nmsBoxNum);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "sample_gen_ovd_cpu_postprocess_multiclass_nms failed!\n");
    return ret;
}

static td_s32 sample_gen_ovd_cpu_postprocess_filter_box(td_float* output, size_t len, td_u32 boxNum)
{
    if (len <= 0) {
        sample_svp_trace_err("output len is 0, sample_gen_ovd_cpu_postprocess_filter_box failed!\n");
    }

    td_u32 boxOffset = 7680; // 7680 is offset of x1y2x2y2
    td_u32 valid_box_num = 0;
    td_float bbox[g_bbox_new_size];
    (td_void)memset_s(g_valid_box, sizeof(g_valid_box), 0, sizeof(g_valid_box));
    (td_void)memset_s(g_save_box, sizeof(g_save_box), 0, sizeof(g_save_box));
    for (td_u32 inx = 0; inx < boxNum; inx++) {
        for (td_u32 i = 0; i < (td_u32)g_bbox_size; i++) {
            bbox[i] = output[inx * g_bbox_size + i];
        }
        bbox[g_bbox_size] = inx;
        td_float centerX = bbox[g_top_left_x];
        td_float centerY = bbox[g_top_left_y];
        td_float boxW = bbox[g_bottom_right_x];
        td_float boxH = bbox[g_bottom_right_y];

        bbox[g_top_left_x] = centerX - boxW / 2; // 2 : half
        bbox[g_top_left_y] = centerY - boxH / 2; // 2 : half
        bbox[g_bottom_right_x] = centerX + boxW / 2; // 2 : half
        bbox[g_bottom_right_y] = centerY + boxH / 2; // 2 : half

        g_save_box[valid_box_num * g_bbox_new_size + g_top_left_x] = bbox[g_top_left_x];
        g_save_box[valid_box_num * g_bbox_new_size + g_top_left_y] = bbox[g_top_left_y];
        g_save_box[valid_box_num * g_bbox_new_size + g_bottom_right_x] = bbox[g_bottom_right_x];
        g_save_box[valid_box_num * g_bbox_new_size + g_bottom_right_y] = bbox[g_bottom_right_y];
        g_save_box[valid_box_num * g_bbox_new_size + g_score] = bbox[g_score];
        g_save_box[valid_box_num * g_bbox_new_size + g_class_id] = bbox[g_class_id];
        g_save_box[valid_box_num * g_bbox_new_size + g_bbox_size] = bbox[g_bbox_size];

        for (td_u32 j = 0; j < (td_u32)g_bbox_size - 2; j++) { // 2 class + score
            bbox[j] += boxOffset * bbox[g_class_id];
        }
        g_valid_box[valid_box_num * g_bbox_new_size + g_top_left_x] = bbox[g_top_left_x];
        g_valid_box[valid_box_num * g_bbox_new_size + g_top_left_y] = bbox[g_top_left_y];
        g_valid_box[valid_box_num * g_bbox_new_size + g_bottom_right_x] = bbox[g_bottom_right_x];
        g_valid_box[valid_box_num * g_bbox_new_size + g_bottom_right_y] = bbox[g_bottom_right_y];
        g_valid_box[valid_box_num * g_bbox_new_size + g_score] = bbox[g_score];
        g_valid_box[valid_box_num * g_bbox_new_size + g_class_id] = bbox[g_class_id];
        g_valid_box[valid_box_num * g_bbox_new_size + g_bbox_size] = bbox[g_bbox_size]; // 6 store box original index
        valid_box_num++;
    }

    td_s32 ret = sample_svp_npu_sort_by_box_score(g_valid_box, &valid_box_num, g_bbox_new_size, g_score);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "sample_svp_npu_sort_by_box_score failed!\n");
    ret = sample_gen_ovd_cpu_postprocess_select_topk_box_into_nms(valid_box_num);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "sample_gen_ovd_cpu_postprocess_select_topk_box_into_nms failed!\n");
    return ret;
}

static td_s32 sample_gen_ovd_cpu_postprocess_info(const svp_acl_mdl_io_dims *output_dims,
    td_float* out_data_buffer_addr, td_u64 w_stride_offset)
{
    td_u32 outHeight = output_dims->dims[output_dims->dim_count - 2] - 4; // 2: dim, 4: xywh
    td_s32 ret = (outHeight == CLASS_NUM) ? TD_SUCCESS : TD_FAILURE;
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "output height of om is %u, CLASS_NUM is %u, Please modify CLASS_NUM = %u in sample_svp_npu_process.h file!\n",
        outHeight, CLASS_NUM, outHeight);

    td_float* out_data = out_data_buffer_addr;
    td_float output[g_bbox_size * w_stride_offset];
    (td_void)memset_s(output, sizeof(output), 0, sizeof(output));
    td_u32 boxNum = 0;
    for (td_u64 i = 0; i < CLASS_NUM; i++) {
        for (td_u64 j = 0; j < w_stride_offset; j++) {
            if (out_data[(g_score + i) * w_stride_offset + j] > g_low_score_threshold) {
                output[boxNum * g_bbox_size + g_top_left_x] = out_data[g_top_left_x * w_stride_offset + j];
                output[boxNum * g_bbox_size + g_top_left_y] = out_data[g_top_left_y * w_stride_offset + j];
                output[boxNum * g_bbox_size + g_bottom_right_x] = out_data[g_bottom_right_x * w_stride_offset + j];
                output[boxNum * g_bbox_size + g_bottom_right_y] = out_data[g_bottom_right_y * w_stride_offset + j];
                output[boxNum * g_bbox_size + g_score] = out_data[(g_score + i) * w_stride_offset + j];
                output[boxNum * g_bbox_size + g_class_id] = i;
                boxNum++;
            }
        }
    }

    ret = sample_gen_ovd_cpu_postprocess_filter_box(output, sizeof(output), boxNum);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "sample_gen_ovd_cpu_postprocess_filter_box failed!\n");
    return ret;
}

static td_s32 sample_svp_npu_post_process_ovd(const sample_svp_npu_task_info *task)
{
    svp_acl_error ret = TD_SUCCESS;
    svp_acl_data_buffer *data_buffer = TD_NULL;
    td_void *data = TD_NULL;
    td_u64 w_stride_offset;
    if (task->cfg.is_cached == TD_TRUE) {
        data_buffer = svp_acl_mdl_get_dataset_buffer(task->output_dataset, 0);
        sample_svp_check_exps_return(data_buffer == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th output data_buffer is NULL!\n", 0);

        svp_acl_mdl_io_dims input_dims;
        sample_svp_npu_model_info* model_info = sample_common_svp_npu_get_model_info(task->cfg.model_idx);
        svp_acl_mdl_desc *desc_ptr =  model_info->model_desc;
        ret = svp_acl_mdl_get_input_dims(desc_ptr, 0, &input_dims);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get input dims failed!\n");
        g_input_height = input_dims.dims[input_dims.dim_count - 2]; // 2: dim
        g_input_width = input_dims.dims[input_dims.dim_count - 1];

        svp_acl_mdl_io_dims output_dims;
        ret = svp_acl_mdl_get_output_dims(desc_ptr, 0, &output_dims);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get output dims failed!\n");
        data = svp_acl_get_data_buffer_addr(data_buffer);
        sample_svp_check_exps_return(data == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th output data is NULL!\n", 0);
        w_stride_offset = (td_u64)(svp_acl_mdl_get_output_default_stride(desc_ptr, 0) / sizeof(td_float));
        ret = sample_gen_ovd_cpu_postprocess_info(&output_dims, (td_float*)data, w_stride_offset);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "sample_gen_ovd_cpu_postprocess_info failed!\n");
    }
    return ret;
}

static td_void sample_init_ovd_box_info_for_vgs(ot_sample_svp_rect_info *rect_info,
    const ot_video_frame_info *proc_frame, const ot_video_frame_info *show_frame)
{
    for (td_u32 i = 0; i < rect_info->num; i++) {
        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_LEFT_TOP].x =
            (td_u32)((td_float)g_res_box[i * g_bbox_size + g_top_left_x] /
            proc_frame->video_frame.width * show_frame->video_frame.width) & (~1);

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_LEFT_TOP].y =
            (td_u32)((td_float)g_res_box[i * g_bbox_size + g_top_left_y] /
            proc_frame->video_frame.height * show_frame->video_frame.height) & (~1);

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_RIGHT_TOP].x =
            (td_u32)((td_float)g_res_box[i * g_bbox_size + g_bottom_right_x] /
            proc_frame->video_frame.width * show_frame->video_frame.width) & (~1);

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_RIGHT_TOP].y =
            rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_LEFT_TOP].y;

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_RIGHT_BOTTOM].x =
            rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_RIGHT_TOP].x;

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_RIGHT_BOTTOM].y =
            (td_u32)((td_float)g_res_box[i * g_bbox_size + g_bottom_right_y] /
            proc_frame->video_frame.height * show_frame->video_frame.height) & (~1);

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_LEFT_BOTTOM].x =
            rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_LEFT_TOP].x;

        rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_LEFT_BOTTOM].y =
            rect_info->rect[i].point[SAMPLE_SVP_NPU_RECT_RIGHT_BOTTOM].y;

        rect_info->rect[i].score = g_res_box[i * g_bbox_size + g_score];
        rect_info->rect[i].class_id = (td_u32)g_res_box[i * g_bbox_size + g_class_id];

#if CONFIG_V110
        rect_info->ids[i] = (td_u32)g_res_box[i * g_bbox_size + g_class_id];
#endif
    }
}

static td_s32 sample_svp_npu_ovd_acl_frame_proc(const ot_video_frame_info *ext_frame,
    const ot_video_frame_info *base_frame, td_void *args)
{
    td_s32 ret;
    td_void *virt_addr = TD_NULL;
    td_u32 size = (td_u32)(ext_frame->video_frame.height * ext_frame->video_frame.stride[0] *
        SAMPLE_SVP_NPU_IMG_THREE_CHN / SAMPLE_SVP_NPU_DOUBLE);

    virt_addr = g_svp_npu_vb_virt_addr +
        (ext_frame->video_frame.phys_addr[0] - g_svp_npu_vb_pool_info.pool_phy_addr);
    ret = sample_common_svp_npu_update_input_data_buffer_info(virt_addr, size,
        ext_frame->video_frame.stride[0], 0, &g_svp_npu_task[0]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "update data buffer failed!\n");

    ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[0]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "model execute failed!\n");

    ret = sample_svp_npu_post_process_ovd(&g_svp_npu_task[0]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "ovd postprocess failed!\n");

    /* Writeout ovd result into rect info by VGS */
    g_svp_npu_rect_info.num = g_top_box_num;
    sample_init_ovd_box_info_for_vgs(sample_svp_npu_get_svp_rect_info(), ext_frame, base_frame);

    /* VGS Draw Box */
    ret = sample_common_svp_vgs_fill_rect(base_frame, sample_svp_npu_get_svp_rect_info(), SAMPLE_SVP_NPU_RECT_COLOR);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "vgs fill rect failed!\n");

#if CONFIG_V110
    /* VGS Write id */
    ret = sample_common_svp_vgs_fill_tracker_id(base_frame, sample_svp_npu_get_svp_rect_info(),
        base_frame->video_frame.width, base_frame->video_frame.height, g_osd_buf);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "notify box fill rect failed!\n");
#endif
    return ret;
}

static td_s32 sample_svp_npu_global_get_mmz_addr(td_u8 **ptr, td_u32 size)
{
    sample_svp_check_exps_return(size + g_svp_npu_mmz_alloc_base_offset > SAMPLE_SVP_NPU_OVD_PREALLOC_MMZ, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "sample svp npu malloc memory failed!\n");
    *ptr = g_svp_npu_mmz_alloc_base_attr + g_svp_npu_mmz_alloc_base_offset;
    g_svp_npu_mmz_alloc_base_offset += size;
    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_ovd_init_inside_frame_globals()
{
    td_s32 ret = sample_svp_npu_global_get_mmz_addr((td_u8 **) &g_osd_buf,
        SAMPLE_SVP_NPU_VGS_RGN_CNT * sizeof(ot_vgs_osd));
    sample_svp_check_exps_return(ret != TD_SUCCESS || g_osd_buf == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "vgs osd buf alloc failed!!");
    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_init_tmp_data()
{
    td_s32 ret = svp_acl_rt_malloc_cached(&g_svp_npu_mmz_alloc_base_attr, SAMPLE_SVP_NPU_OVD_PREALLOC_MMZ,
        SVP_ACL_MEM_MALLOC_NORMAL_ONLY); /* Malloc Base Size */
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "malloc cache failed\n");

    /* Malloc Tmp Params Used inside Single Frame */
    ret = sample_svp_npu_ovd_init_inside_frame_globals();
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "init inside frame failed\n");

    return TD_SUCCESS;
}

static td_void *sample_svp_npu_ovd_acl_pc(td_void *args)
{
    td_u32 size, stride;
    td_u8 *data = TD_NULL;

    td_s32 ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "open device failed!\n");

    ret = sample_common_svp_npu_get_input_data_buffer_info(&g_svp_npu_task[0], 0, &data, &size, &stride);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, fail_0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),get_input0_data_buffer_info failed!\n", ret);

    ret = svp_acl_rt_malloc_cached(&g_svp_npu_mmz_alloc_base_attr, SAMPLE_SVP_NPU_OVD_PREALLOC_MMZ,
        SVP_ACL_MEM_MALLOC_NORMAL_ONLY); /* Malloc Base Size */
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "malloc cache failed\n");

    ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[0]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "model execute failed!\n");

    ret = sample_svp_npu_post_process_ovd(&g_svp_npu_task[0]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "ovd postprocess failed!\n");

fail_0:
    (td_void)svp_acl_rt_reset_device(g_svp_npu_dev_id);
    return TD_NULL;
}

static td_void *sample_svp_npu_ovd_acl_vdec_to_vo(td_void *args)
{
    ot_video_frame_info base_frame, ext_frame;
    const td_s32 milli_sec = SAMPLE_SVP_NPU_MILLIC_SEC;
    const ot_vo_layer vo_layer = 0;
    const ot_vo_chn vo_chn = 0;
    const td_s32 vpss_grp = 0;
    td_s32 vpss_chn[] = { OT_VPSS_CHN0, OT_VPSS_CHN1 };
    td_u32 size, stride;
    td_u8 *data = TD_NULL;

    (td_void)prctl(PR_SET_NAME, "svp_npu_vdec_to_vo", 0, 0, 0);

    td_s32 ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "open device failed!\n");

    ret = sample_svp_npu_acl_vb_map(OT_VPSS_CHN1);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, fail_0, SAMPLE_SVP_ERR_LEVEL_ERROR, "map vb pool failed!\n");

    ret = sample_common_svp_npu_get_input_data_buffer_info(&g_svp_npu_task[0], 0, &data, &size, &stride);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, fail_1, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),get_input0_data_buffer_info failed!\n", ret);

    ret = sample_svp_npu_init_tmp_data();
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR, "init tmp data failed!\n");

    while (g_svp_npu_thread_stop == TD_FALSE) {
        ret = ss_mpi_vpss_get_chn_frame(vpss_grp, vpss_chn[1], &ext_frame, milli_sec);
        sample_svp_check_exps_continue(ret != TD_SUCCESS, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),ss_mpi_vpss_get_chn_frame failed, vpss_grp(%d), vpss_chn(%d)!\n", ret, vpss_grp, vpss_chn[1]);

        ret = ss_mpi_vpss_get_chn_frame(vpss_grp, vpss_chn[0], &base_frame, milli_sec);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, ext_release, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),ss_mpi_vpss_get_chn_frame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n", ret, vpss_grp, vpss_chn[0]);

        ret = sample_svp_npu_ovd_acl_frame_proc(&ext_frame, &base_frame, args);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, base_release, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),sample_svp_npu_ovd_acl_frame_proc failed!\n", ret);

        ret = sample_common_svp_venc_vo_send_stream(&g_svp_npu_media_cfg.svp_switch, 0, vo_layer, vo_chn, &base_frame);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, base_release, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),sample_common_svp_venc_vo_send_stream failed!\n", ret);
base_release:
        ret = ss_mpi_vpss_release_chn_frame(vpss_grp, vpss_chn[0], &base_frame);
        sample_svp_check_exps_trace(ret != TD_SUCCESS, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),release_frame failed,grp(%d) chn(%d)!\n", ret, vpss_grp, vpss_chn[0]);
ext_release:
        ret = ss_mpi_vpss_release_chn_frame(vpss_grp, vpss_chn[1], &ext_frame);
        sample_svp_check_exps_trace(ret != TD_SUCCESS, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),release_frame failed,grp(%d) chn(%d)!\n", ret, vpss_grp, vpss_chn[1]);
    }
    ret = sample_common_svp_npu_update_input_data_buffer_info(data, size, stride, 0, &g_svp_npu_task[0]);
    sample_svp_check_exps_trace(ret != TD_SUCCESS, SAMPLE_SVP_ERR_LEVEL_ERROR, "update buffer failed!\n");
fail_1:
    (td_void)ss_mpi_sys_munmap(g_svp_npu_vb_virt_addr, g_svp_npu_vb_pool_info.pool_size);
fail_0:
    (td_void)svp_acl_rt_reset_device(g_svp_npu_dev_id);
    return TD_NULL;
}

static td_s32 sample_svp_npu_init_media(td_u32 is_file)
{
    td_s32 ret;
    if (is_file) {
        /* start vdec vpss venc vo [File] */
        ret = sample_common_svp_create_vb_start_vdec_vpss_vo(&g_svp_npu_vdec_cfg, &g_svp_npu_vdec_param,
            &g_svp_npu_vdec_thread, &g_svp_npu_media_cfg, &g_svp_npu_vo_cfg);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_DEBUG, "init media failed!\n");
    } else {
        /* start vi vpss vo: [Camera] */
        ret = sample_common_svp_npu_start_vi_vpss_vo(&g_vi_config, &g_svp_npu_media_cfg, &g_svp_npu_vo_cfg);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_DEBUG, "init media failed!\n");
    }
    return ret;
}

static td_s32 sample_svp_check_task_cfg(const sample_svp_npu_task_info *task)
{
    sample_svp_npu_model_info *model_info = TD_NULL;

    sample_svp_check_exps_return(task == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "task is NULL!\n");

    sample_svp_check_exps_return(task->cfg.max_batch_num == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "max_batch_num(%u) is 0!\n", task->cfg.max_batch_num);

    sample_svp_check_exps_return(task->cfg.dynamic_batch_num == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "dynamic_batch_num(%u) is 0!\n", task->cfg.dynamic_batch_num);

    sample_svp_check_exps_return(task->cfg.total_t != 0 && task->cfg.dynamic_batch_num != 1, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "dynamic_batch_num(%u) should be 1 when total_t(%u) is not 0!\n",
        task->cfg.dynamic_batch_num, task->cfg.total_t);

    sample_svp_check_exps_return((task->cfg.is_cached != TD_TRUE && task->cfg.is_cached != TD_FALSE), TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "is_cached(%u) should be [%u, %u]!\n", task->cfg.is_cached, TD_FALSE, TD_TRUE);

    model_info = sample_common_svp_npu_get_model_info(task->cfg.model_idx);

    sample_svp_check_exps_return(task->cfg.model_idx >= SAMPLE_SVP_NPU_MAX_MODEL_NUM, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "model_idx(%u) should be less than %u!\n",
        task->cfg.model_idx, SAMPLE_SVP_NPU_MAX_MODEL_NUM);

    sample_svp_check_exps_return(model_info->model_desc == TD_NULL, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th model_desc is NULL!\n", task->cfg.model_idx);
    return TD_SUCCESS;
}

static td_void sort_class_id(td_u32 arr[], td_u32 n)
{
    td_u32 i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                td_u32 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

td_s32 ovd_second_input_pre_process(const sample_svp_npu_task_info *task)
{
    td_s32 ret = TD_SUCCESS;
    const td_char *class_list_file = "./data/class_lists.txt";
    td_char class_list_txt[CLASS_NUM][NAME_LEN];
    ret = read_file(class_list_file, class_list_txt);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "read class_lists.txt failed!\n");

    svp_acl_mdl_io_dims output_dims;
    sample_svp_npu_model_info* model_info = sample_common_svp_npu_get_model_info(task->cfg.model_idx);
    svp_acl_mdl_desc *desc_ptr =  model_info->model_desc;
    ret = svp_acl_mdl_get_output_dims(desc_ptr, 0, &output_dims);
    sample_svp_check_exps_return(ret != TD_SUCCESS || output_dims.dim_count < 2, TD_FAILURE, // 2: dim count
        SAMPLE_SVP_ERR_LEVEL_ERROR, "get output dims fail!\n");

    td_u32 outHeight = output_dims.dims[output_dims.dim_count - 2] - 4; // 2: dim, 4: xywh
    ret = (outHeight == lineCount) ? TD_SUCCESS : TD_FAILURE;
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "output height of om is %u, class_lists.txt class num is %u, these two must be consistent!\n",
        outHeight, lineCount);

    const td_char *class_config_file = "./sample_svp_npu/text_config.txt";
    td_char class_config_txt[CLASS_NUM][NAME_LEN];
    ret = read_file(class_config_file, class_config_txt);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "read text_config.txt failed!\n");

    // Search for the location of the class name in class_config_txt
    td_u32 id = 0;
    for (td_u32 i = 0; i < lineCount; i++) {
        class_config_txt[i][strcspn(class_config_txt[i], "\n")] = '\0';
        for (td_u32 j = 0; j < CLASS_NUM; j++) {
            class_list_txt[j][strcspn(class_list_txt[j], "\n")] = '\0';
            if (!strcmp(class_config_txt[i], class_list_txt[j])) {
                inputClassId[id] = j;
                id++;
                break;
            }
        }
    }

    // sort inputClassId
    sort_class_id(inputClassId, id);

    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_get_ovd_second_input_data(const td_char *src, const sample_svp_npu_task_info *task)
{
    sample_svp_check_exps_return(ovd_second_input_pre_process(task) != TD_SUCCESS, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "ovd second input pre process failed!\n");

    td_char path[PATH_MAX + 1] = { 0 };
    FILE *fp = TD_NULL;
    td_void *data = TD_NULL;
    svp_acl_data_buffer *data_buffer = TD_NULL;

    sample_svp_check_exps_return(sample_svp_check_task_cfg(task) != TD_SUCCESS, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "check task cfg failed!\n");

    sample_svp_check_exps_return(realpath(src, path) == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error sample_common_svp_npu_get_ovd_second_input_data, Invalid %s file or path!\n", src);

    fp = fopen(path, "rb");
    sample_svp_check_exps_return(fp == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "open file failed!\n");

    data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, 1);
    sample_svp_check_exps_goto(data_buffer == TD_NULL, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data buffer NULL!\n");

    data = svp_acl_get_data_buffer_addr(data_buffer);
    sample_svp_check_exps_goto(data == TD_NULL, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data addr NULL!\n");

    size_t stride = svp_acl_get_data_buffer_stride(data_buffer);
    sample_svp_check_exps_goto(stride == 0, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data stride failed!\n");

    size_t size = svp_acl_get_data_buffer_size(data_buffer);
    td_u32 total_line_num = size / stride;
    sample_svp_check_exps_goto(total_line_num != CLASS_NUM, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "total_line_num fail!\n");
    td_u32 line_byte_num = stride;
    sample_svp_check_exps_goto(size < (td_u64)task->cfg.dynamic_batch_num * total_line_num * stride, end,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th data buffer size(%zu) is less than needed(%llu)!\n",
        1, size, (td_u64)task->cfg.dynamic_batch_num * total_line_num * stride);

    // InitData
    for (td_u32 j = 0; j < task->cfg.dynamic_batch_num * total_line_num; j++) {
        (td_void)memset_s(data + j * stride, line_byte_num, 0, line_byte_num);
    }

     // FillData
    td_u32 ret, l = 0;
    for (td_u32 line = 0; line < task->cfg.dynamic_batch_num * total_line_num; line++) {
        if (inputClassId[l] == line) {
            ret = fread(data + line * stride, line_byte_num, 1, fp);
            sample_svp_check_exps_goto(ret != 1, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Read file failed!\n");
            l++;
        } else {
            fseek(fp, line_byte_num, SEEK_CUR);
        }
    }

    if (task->cfg.is_cached == TD_TRUE) {
        (td_void)svp_acl_rt_mem_flush(data, task->cfg.dynamic_batch_num * total_line_num * stride);
    }
    (td_void)fclose(fp);
    return TD_SUCCESS;

end:
    if (fp != TD_NULL) {
        (td_void)fclose(fp);
    }
    return TD_FAILURE;
}

td_s32 read_file(const td_char *src, td_char class[CLASS_NUM][NAME_LEN])
{
    FILE *fp = TD_NULL;
    td_char path[PATH_MAX + 1] = { 0 };
    td_char line[NAME_LEN];

    sample_svp_check_exps_return(src == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "src is NULL!\n");
    sample_svp_check_exps_return(realpath(src, path) == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error read_file, Invalid %s file or path!\n", src);

    // check text_config.txt
    fp = fopen(path, "rb");
    sample_svp_check_exps_goto(fp == TD_NULL, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "open file failed!\n");
    td_s32 ret = TD_SUCCESS;
    td_s32 lineCounts = 0;
    while (fgets(line, NAME_LEN, fp) != NULL) {
        lineCounts++;
    }
    sample_svp_check_exps_goto(lineCounts > CLASS_NUM, end, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "class_num[%d] exceeds %d in the %s , please reconfigure the file!\n", lineCounts, CLASS_NUM, path);
    (td_void)fclose(fp);

    // read class_lists.txt or text_config.txt
    fp = fopen(path, "rb");
    lineCount = 0;
    sample_svp_check_exps_goto(fp == TD_NULL, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "open file failed!\n");
    while (fgets(line, NAME_LEN, fp) != NULL) {
        ret = strcpy_s(class[lineCount], NAME_LEN, line);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, end, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "copy the file content failed!\n");
        lineCount++;
    }
    (td_void)fclose(fp);
    return TD_SUCCESS;

end:
    if (fp != TD_NULL) {
        (td_void)fclose(fp);
    }
    return TD_FAILURE;
}

static td_s32 sample_ovd_acl_init(td_bool is_file)
{
    td_s32 ret = TD_SUCCESS;
    /* use PIC_BUTT to be a flag, get the input resolution form om */
    g_svp_npu_media_cfg.pic_type[1] = PIC_BUTT;
    g_svp_npu_terminate_signal = TD_FALSE;

    /* init acl */
#if CONFIG_V110
    ret = sample_svp_npu_acl_init(TD_NULL);
#else
    if (is_file) {
        ret = sample_svp_npu_acl_init(TD_NULL, TD_FALSE);
    } else {
        ret = sample_svp_npu_acl_init(TD_NULL, TD_TRUE);
    }
#endif
    return ret;
}

td_void sample_svp_npu_acl_ovd_board(td_u32 is_file)
{
    td_s32 ret;
    const td_u32 model_idx = 0;
    sample_svp_npu_thread_args args = {0};

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        /* init acl */
        ret = sample_ovd_acl_init(is_file);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

        /* load model */
        td_char path[PATH_MAX + 1] = { 0 };
        const td_char *om_model_path = "./model/hisi_ovd_v1.0.0_coco_46.1_lvis_30.3.om";
        sample_svp_check_exps_goto(realpath(om_model_path, path) == TD_NULL, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error sample_svp_npu_acl_ovd_board, Invalid %s file or path!\n", om_model_path);
        ret = sample_common_svp_npu_load_model(path, model_idx, TD_FALSE);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "load model failed!\n");

        /* get input resolution */
        ret = sample_common_svp_npu_get_input_resolution(model_idx, 0, &g_svp_npu_media_cfg.pic_size[1]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get pic size failed!\n");

        /* init media */
        sample_svp_npu_init_media(is_file);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_DEBUG, "media init fail!\n");

        /* set cfg */
        sample_svp_npu_acl_set_task_info(0, model_idx);

        ret = sample_svp_npu_acl_init_task(1, TD_FALSE, 0);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "init task failed!\n");
    }

    /* process */
    if (g_svp_npu_terminate_signal == TD_FALSE) {
        const td_char *src_file = "./data/text_features.bin";
        ret = sample_common_svp_npu_get_ovd_second_input_data(src_file, &g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data failed!\n");

        g_svp_npu_thread_stop = TD_FALSE;
        ret = pthread_create(&g_svp_npu_thread, 0, sample_svp_npu_ovd_acl_vdec_to_vo, (td_void*)&args);
        sample_svp_check_exps_goto(ret != 0, process_end4, SAMPLE_SVP_ERR_LEVEL_ERROR, "create thread failed!\n");

        (td_void)sample_svp_npu_acl_pause();
        g_svp_npu_thread_stop = TD_TRUE;
        pthread_join(g_svp_npu_thread, TD_NULL);
    }
process_end4:
    (td_void)svp_acl_rt_free(g_svp_npu_mmz_alloc_base_attr);
process_end3:
    (td_void)sample_svp_npu_acl_deinit_task(1, 0);
process_end2:
    if (is_file) {
        (td_void)sample_common_svp_destroy_vb_stop_vdec_vpss_vo(&g_svp_npu_vdec_param, &g_svp_npu_vdec_thread,
            &g_svp_npu_media_cfg, &g_svp_npu_vo_cfg);
    } else {
        (td_void)sample_common_svp_destroy_vb_stop_vi_vpss_vo(&g_vi_config, &g_svp_npu_media_cfg, &g_svp_npu_vo_cfg);
    }
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

td_void sample_svp_npu_acl_ovd_pc(const td_char *img_name)
{
    g_is_pc_sample = TD_TRUE;
    td_s32 ret = TD_SUCCESS;
    const td_u32 model_idx = 0;
    td_u32 is_file = 0;
    sample_svp_npu_thread_args args = {0};
    td_char img_file[1024];
    td_char buffer[512];

    /* init acl */
    ret = sample_ovd_acl_init(is_file);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

    /* load model */
    td_char path[PATH_MAX + 1] = { 0 };
    const td_char *om_model_path = "./model/hisi_ovd_v1.0.0_coco_46.1_lvis_30.3.om";
    sample_svp_check_exps_goto(realpath(om_model_path, path) == TD_NULL, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error sample_svp_npu_acl_ovd_board, Invalid %s file or path!\n", om_model_path);
    ret = sample_common_svp_npu_load_model(path, model_idx, TD_FALSE);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR, "load model failed!\n");

    /* set cfg */
    sample_svp_npu_acl_set_task_info(0, model_idx);

    /* init task */
    ret = sample_svp_npu_acl_init_task(1, TD_FALSE, 0);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "init task failed!\n");

    /* default file */
    ret = snprintf_s(img_file, sizeof(img_file), sizeof(img_file), "%s", "./img/binfile/dog_bike_car_ovd.bin");
    sample_svp_check_exps_goto(ret <= 0, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get img_file failed!\n");
    if (strlen(img_name) > 0) {
        const td_char *dot = strchr(img_name, '.');
        ret = strncpy_s(buffer, sizeof(buffer), img_name, dot - img_name);
        sample_svp_check_exps_goto(ret != 0, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get img_name failed!\n");
        ret = snprintf_s(img_file, sizeof(img_file), sizeof(img_file), "%s%s%s", "./img/binfile/", buffer, "_ovd.bin");
        sample_svp_check_exps_goto(ret <= 0, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get img_file failed!\n");
    }
    /* get ovd input data */
    const td_char *first_input_file[SAMPLE_SVP_NPU_OVD_INPUT_FILE_NUM] = {
        img_file, "./data/text_features.bin"
    };
    ret = sample_common_svp_npu_get_input_data(first_input_file, SAMPLE_SVP_NPU_OVD_INPUT_FILE_NUM, &g_svp_npu_task[0]);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data failed!\n");

    /* set ovd second input data */
    const td_char *second_input_file = "./data/text_features.bin";
    ret = sample_common_svp_npu_get_ovd_second_input_data(second_input_file, &g_svp_npu_task[0]);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data failed!\n");

    g_svp_npu_thread_stop = TD_FALSE;
    ret = pthread_create(&g_svp_npu_thread, 0, sample_svp_npu_ovd_acl_pc, (td_void*)&args);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR, "create thread failed!\n");

    g_svp_npu_thread_stop = TD_TRUE;
    pthread_join(g_svp_npu_thread, TD_NULL);

process_end3:
    (td_void)svp_acl_rt_free(g_svp_npu_mmz_alloc_base_attr);
process_end2:
    (td_void)sample_svp_npu_acl_deinit_task(1, 0);
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
}

/* function : show the sample of ovd */
td_void sample_svp_npu_acl_ovd(td_u32 is_file, const td_char *img_name)
{
    switch (is_file) {
        case 0:
        case 1:
            sample_svp_npu_acl_ovd_board(is_file);
            break;
        case 2: // 2: pc sample
            sample_svp_npu_acl_ovd_pc(img_name);
        default:
            break;
    }
}
