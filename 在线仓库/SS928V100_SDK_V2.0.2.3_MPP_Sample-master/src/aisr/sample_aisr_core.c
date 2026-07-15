/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include "sample_aisr_core.h"
#include <limits.h>
#include <unistd.h>
#include "ot_common_vb.h"
#include "ot_common_video.h"
#include "ot_buffer.h"
#include "ot_common_aiv.h"
#include "ss_mpi_aisr.h"
#include "sample_comm.h"

#define AISR_NET_NUM 1
#define INPUT_YUV_WIDTH 1920
#define INPUT_YUV_HEIGHT 1080
#define MODEL_FILE_NAME "./aisr_model.bin"

static ot_aisr_model g_aisr_model[AISR_NET_NUM] = {0};
static td_s32 g_aisr_model_id[AISR_NET_NUM] = {[0 ... (AISR_NET_NUM - 1)] = -1};

td_bool g_thread_start = TD_TRUE;
typedef struct {
    td_s32 chn_id;
    ot_size in_size;
    const td_char *input_yuv_file;
} thread_params;

static td_void sample_aisr_get_buffer_size(ot_size in_size, ot_pixel_format pixel_format, ot_vb_calc_cfg *calc_cfg)
{
    ot_pic_buf_attr buf_attr = { 0 };

    buf_attr.width = in_size.width;
    buf_attr.height = in_size.height;
    buf_attr.align = OT_DEFAULT_ALIGN;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format = pixel_format;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    ot_common_get_pic_buf_cfg(&buf_attr, calc_cfg);
}

static td_s32 sample_aisr_sys_init(ot_size in_size, td_u32 vb_cnt)
{
    td_s32 ret;
    ot_vb_cfg vb_cfg = { 0 };
    ot_vb_calc_cfg calc_cfg = { 0 };

    in_size.width = INPUT_YUV_WIDTH;
    in_size.height = INPUT_YUV_HEIGHT;
    sample_aisr_get_buffer_size(in_size, OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422, &calc_cfg);

    vb_cfg.max_pool_cnt = 128; /* 128 blks */

    sample_print("aisr set vb blk size: %d.\n", calc_cfg.vb_size);
    vb_cfg.common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg.common_pool[0].blk_cnt = vb_cnt;

    ret = sample_comm_sys_init(&vb_cfg);
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_FAILURE, "sample common sys init failed.\n");

    return TD_SUCCESS;
}

static td_void sample_aisr_free_mem(ot_aiv_mem_info *mem)
{
    ss_mpi_sys_mmz_free(mem->phys_addr, mem->virt_addr);
    mem->phys_addr = 0;
    mem->virt_addr = TD_NULL;
}

static td_u32 sample_aisr_get_file_size(FILE *fp)
{
    td_s32 ret;
    td_u32 file_size;

    sample_aisr_check_exps_return((fp == TD_NULL), 0, "fp nullptr!\n");

    ret = fseek(fp, 0L, SEEK_END);
    sample_aisr_check_exps_return((ret != 0), 0, "fseek end failed!\n");

    file_size = ftell(fp);
    sample_aisr_check_exps_return((file_size <= 0), 0, "ftell failed!\n");

    ret = fseek(fp, 0L, SEEK_SET);
    sample_aisr_check_exps_return((ret != 0), 0, "fseek set failed!\n");

    return file_size;
}

static td_s32 sample_aisr_common_load_mem(td_char *model_file_name, ot_aiv_mem_info *mem)
{
    td_s32 ret;
    size_t fret;
    FILE *fp = TD_NULL;

    /* Get model file size */
    fp = fopen(model_file_name, "rb");
    sample_aisr_check_exps_return((fp == TD_NULL), TD_FAILURE, "fopen file %s err!\n", model_file_name);

    /* malloc model file mem */
    mem->size = sample_aisr_get_file_size(fp);
    ret = ss_mpi_sys_mmz_alloc(&(mem->phys_addr), &(mem->virt_addr), "aisr_model_sample", TD_NULL, mem->size);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), fail_0, "ss_mpi_sys_mmz_alloc failed!\n");

    fret = fread(mem->virt_addr, mem->size, 1, fp);
    sample_aisr_check_exps_goto((fret != 1), fail_1, "read model file failed!\n");

    (td_void)fclose(fp);
    return TD_SUCCESS;

fail_1:
    sample_aisr_free_mem(mem);
fail_0:
    (td_void)fclose(fp);

    return TD_FAILURE;
}

static td_void sample_aisr_common_unload_mem(ot_aiv_mem_info *param_mem)
{
    if ((param_mem->phys_addr != 0) && (param_mem->virt_addr != TD_NULL)) {
        (td_void)ss_mpi_sys_mmz_free(param_mem->phys_addr, param_mem->virt_addr);
    }
}

static td_void sample_aisr_unload_model(td_void)
{
    td_u32 i;

    for (i = 0; i < AISR_NET_NUM; i++) {
        if (g_aisr_model_id[i] == -1) {
            continue;
        }
        ss_mpi_aisr_unload_model(g_aisr_model_id[i]);
        g_aisr_model_id[i] = -1;
    }
}

static td_s32 sample_aisr_load_model(ot_size in_size)
{
    td_u32 i;
    td_s32 ret;
    ot_aisr_model *model_cfg = TD_NULL;

    for (i = 0; i < AISR_NET_NUM; i++) {
        model_cfg = &g_aisr_model[i];
        ret = sample_aisr_common_load_mem(MODEL_FILE_NAME, &model_cfg->model.mem_info);
        sample_aisr_check_exps_goto((ret != TD_SUCCESS), err_exit, "sample_aisr_common_load_mem %u error\n", i);

        model_cfg->model.image_size.width = in_size.width;
        model_cfg->model.image_size.height = in_size.height;
        ret = ss_mpi_aisr_load_model(model_cfg, &g_aisr_model_id[i]);
        sample_aisr_check_exps_goto((ret != TD_SUCCESS), err_exit, "ss_mpi_aisr_load_model %u error\n", i);

        sample_aisr_common_unload_mem(&model_cfg->model.mem_info);
    }

    return TD_SUCCESS;

err_exit:
    sample_aisr_unload_model();

    return TD_FAILURE;
}

static td_s32 sample_aisr_start(td_s32 chn_id, ot_size in_size)
{
    td_s32 ret;
    ot_aisr_cfg cfg = { 0 };
    ot_aisr_attr attr = { 0 };

    ret = ss_mpi_aisr_init();
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_FAILURE, "ss_mpi_aisr_init failed.\n");

    ret = sample_aisr_load_model(in_size);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), task_exit, "sample_aisr_load_model failed.\n");

    cfg.size.width = in_size.width;
    cfg.size.height = in_size.height;

    ret = ss_mpi_aisr_set_cfg(chn_id, &cfg);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), task_exit, "ss_mpi_aisr_set_cfg failed.\n");

    ret = ss_mpi_aisr_enable(chn_id);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), unload_cfg, "ss_mpi_aisr_enable failed.\n");

    attr.enable = TD_TRUE;
    attr.input_depth = 6;  // 6: default input frame queue depth
    attr.output_depth = 6; // 6: default input frame queue depth
    ret = ss_mpi_aisr_set_attr(chn_id, &attr);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), disable, "sample_aisr_set_attr failed.\n");

    return TD_SUCCESS;

disable:
    ss_mpi_aisr_disable(chn_id);
unload_cfg:
    sample_aisr_unload_model();
task_exit:
    ss_mpi_aisr_exit();

    return TD_FAILURE;
}

static td_s32 sample_aisr_stop(td_s32 chn_id)
{
    td_s32 return_value = TD_SUCCESS;
    td_s32 ret;

    ret = ss_mpi_aisr_disable(chn_id);
    if (ret != TD_SUCCESS) {
        return_value = TD_FAILURE;
        sample_print("ss_mpi_aisr_disable false error(%#x)\n", ret);
    }

    sample_aisr_unload_model();

    ss_mpi_aisr_exit();

    return return_value;
}

static td_s32 sample_aisr_malloc_frame(td_u32 vb_size, ot_video_frame_info *frame)
{
    ot_vb_blk vb_blk;
    td_phys_addr_t phys_addr;
    td_void *virt_addr;

    vb_blk = ss_mpi_vb_get_blk(0, vb_size, "sample_aisr_image");
    sample_aisr_check_exps_return((vb_blk == OT_VB_INVALID_HANDLE), TD_FAILURE, "frame ss_mpi_vb_get_blk failed.\n");

    phys_addr = ss_mpi_vb_handle_to_phys_addr(vb_blk);
    sample_aisr_check_exps_return((phys_addr == 0), TD_FAILURE, "frame ss_mpi_vb_handle_to_phys_addr failed.\n");

    virt_addr = ss_mpi_sys_mmap(phys_addr, vb_size);
    sample_aisr_check_exps_return((virt_addr == TD_NULL), TD_FAILURE, "frame ss_mpi_sys_mmap failed.\n");

    frame->video_frame.phys_addr[0] = phys_addr;
    frame->video_frame.virt_addr[0] = virt_addr;
    return TD_SUCCESS;
}

static td_s32 sample_aisr_free_frame(const ot_video_frame_info *frame)
{
    ot_vb_blk vb_blk;
    td_s32 ret;
    ot_size in_size;
    ot_vb_calc_cfg calc_cfg;

    in_size.width = frame->video_frame.width;
    in_size.height = frame->video_frame.height;
    sample_aisr_get_buffer_size(in_size, frame->video_frame.pixel_format, &calc_cfg);

    ret = ss_mpi_sys_munmap(frame->video_frame.virt_addr[0], calc_cfg.vb_size);
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_FAILURE, "frame ss_mpi_sys_munmap failed.\n");

    vb_blk = ss_mpi_vb_phys_addr_to_handle(frame->video_frame.phys_addr[0]);
    sample_aisr_check_exps_return((vb_blk == OT_VB_INVALID_HANDLE), TD_FAILURE,
        "frame ss_mpi_vb_phys_addr_to_handle\n");

    ret = ss_mpi_vb_release_blk(vb_blk);
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_FAILURE, "frame ss_mpi_vb_release_blk failed.\n");

    return TD_SUCCESS;
}

static td_s32 sample_aisr_read_frame(const td_char *file_name, ot_size in_size, ot_video_frame_info *in_frame)
{
    FILE *fp = TD_NULL;
    td_s32 ret;
    size_t fret;
    td_u32 file_size;
    ot_vb_calc_cfg calc_cfg = { 0 };
    td_char path[PATH_MAX + 1] = {0};

    sample_aisr_get_buffer_size(in_size, OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420, &calc_cfg);

    sample_aisr_check_exps_return((strlen(file_name) > PATH_MAX || realpath(file_name, path) == TD_NULL), TD_FAILURE,
        "check realpath failed.\n");
    fp = fopen(path, "rb");
    sample_aisr_check_exps_return((fp == TD_NULL), TD_FAILURE, "fopen (%s) failed.\n", file_name);

    file_size = sample_aisr_get_file_size(fp);
    sample_aisr_check_exps_goto((file_size != calc_cfg.vb_size), close_file,
        "file size (%u) not equal with buffer size (%d)\n", file_size, calc_cfg.vb_size);

    ret = sample_aisr_malloc_frame(calc_cfg.vb_size, in_frame);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), close_file, "sample_aisr_malloc_frame failed!\n");

    fret = fread(in_frame->video_frame.virt_addr[0], calc_cfg.vb_size, 1, fp);
    sample_aisr_check_exps_goto((fret != 1), mmz_free, "read frame file failed!\n");

    in_frame->mod_id = OT_ID_USER;
    in_frame->video_frame.width = in_size.width;
    in_frame->video_frame.height = in_size.height;
    in_frame->video_frame.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    in_frame->video_frame.stride[0] = calc_cfg.main_stride;
    in_frame->video_frame.stride[1] = calc_cfg.main_stride;
    in_frame->video_frame.phys_addr[1] = in_frame->video_frame.phys_addr[0] + calc_cfg.main_y_size;
    in_frame->video_frame.virt_addr[1] = in_frame->video_frame.virt_addr[0] + calc_cfg.main_y_size;

    (td_void)fclose(fp);
    return TD_SUCCESS;

mmz_free:
    sample_aisr_free_frame(in_frame);
close_file:
    (td_void)fclose(fp);
    return TD_FAILURE;
}

static td_s32 sample_aisr_dump_frame(const ot_video_frame_info *frame, const td_char *save_name)
{
    FILE *fp = TD_NULL;
    size_t fret;
    td_void *virt_addr = TD_NULL;
    ot_vb_calc_cfg calc_cfg = { 0 };

    ot_size frame_size = { frame->video_frame.width, frame->video_frame.height };
    sample_aisr_get_buffer_size(frame_size, frame->video_frame.pixel_format, &calc_cfg);

    fp = fopen(save_name, "wb");
    sample_aisr_check_exps_return((fp == TD_NULL), TD_FAILURE, "fopen (%s) failed.\n", save_name);

    virt_addr = ss_mpi_sys_mmap(frame->video_frame.phys_addr[0], calc_cfg.vb_size);
    sample_aisr_check_exps_goto((virt_addr == TD_NULL), close_file, "ss_mpi_sys_mmap frame failed.\n");

    fret = fwrite(virt_addr, calc_cfg.vb_size, 1, fp);
    sample_aisr_check_exps_goto((fret != 1), umap_frame, "write frame file failed!\n");

    (td_void)ss_mpi_sys_munmap(virt_addr, calc_cfg.vb_size);
    (td_void)fclose(fp);
    return TD_SUCCESS;

umap_frame:
    (td_void)ss_mpi_sys_munmap(virt_addr, calc_cfg.vb_size);
close_file:
    (td_void)fclose(fp);
    return TD_FAILURE;
}

static td_void sample_aisr_release_frame(ot_video_frame_info *in_frame)
{
    sample_aisr_free_frame(in_frame);
}

static td_void *sample_aisr_send_thread(td_void *p)
{
    td_s32 ret = TD_SUCCESS;
    thread_params *params = TD_NULL;
    ot_video_frame_info in_frame_info = { 0 };
    td_u32 send_gap_us = 30 * 1000; // 30: sleep 30ms between send frame; 1000: multiple between ms and us

    params = (thread_params *)p;
    ret = sample_aisr_read_frame(params->input_yuv_file, params->in_size, &in_frame_info);
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_NULL, "sample_aisr_read_frame failed\n");

    while (g_thread_start) {
        ret = ss_mpi_aisr_send_frame(params->chn_id, &in_frame_info, -1);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_aisr_send_frame failed\n");
        }
        usleep(send_gap_us);
    }
    sample_aisr_release_frame(&in_frame_info);

    return TD_NULL;
}

static td_void *sample_aisr_get_thread(td_void *p)
{
    td_s32 ret = TD_SUCCESS;
    td_s32 milli_sec = 1000;
    ot_video_frame_info out_frame_info = { 0 };
    thread_params *params = TD_NULL;

    params = (thread_params *)p;
    while (g_thread_start) {
        ret = ss_mpi_aisr_get_frame(params->chn_id, &out_frame_info, milli_sec);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_aisr_get_frame failed\n");
        }

        ret = sample_aisr_dump_frame(&out_frame_info, "./test_out.yuv");
        if (ret != TD_SUCCESS) {
            sample_print("sample_aisr_dump_frame failed\n");
        }

        ret = ss_mpi_aisr_release_frame(params->chn_id, &out_frame_info);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_aisr_release_frame failed\n");
        }
    }

    return TD_NULL;
}

static td_void sample_aisr_get_char(td_char *s)
{
    printf("---------------press any key to %s!---------------\n", s);
    (td_void)getchar();
}

static td_s32 sample_aisr_process_pic(td_s32 chn_id, ot_size in_size, const td_char *input_yuv_file)
{
    td_s32 ret;
    pthread_t send_thread;
    pthread_t get_thread;
    thread_params proc_thread_param;

    proc_thread_param.chn_id = chn_id;
    proc_thread_param.in_size = in_size;
    proc_thread_param.input_yuv_file = input_yuv_file;

    ret = pthread_create(&send_thread, TD_NULL, sample_aisr_send_thread, &proc_thread_param);
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_FAILURE, "create send pthread failed!\n");

    ret = pthread_create(&get_thread, TD_NULL, sample_aisr_get_thread, &proc_thread_param);
    sample_aisr_check_exps_return((ret != TD_SUCCESS), TD_FAILURE, "create get pthread failed!\n");

    sample_aisr_get_char("exit");
    g_thread_start = TD_FALSE;
    pthread_join(get_thread, TD_NULL);
    pthread_join(send_thread, TD_NULL);
    return TD_SUCCESS;
}

td_s32 sample_aisr_run_pic(const td_char *input_yuv_file)
{
    td_s32 ret = TD_SUCCESS;
    td_s32 chn_id = 0;
    ot_size in_size = { 1920, 1080 }; // 1080p support only

    ret = sample_aisr_sys_init(in_size, 20); // 20 blks
    sample_aisr_check_exps_return((ret != TD_SUCCESS), ret, "sample_aisr_sys_init fail.\n");

    ret = sample_aisr_start(chn_id, in_size);
    sample_aisr_check_exps_goto((ret != TD_SUCCESS), exit1, "sample_aisr_start fail.\n");

    ret = sample_aisr_process_pic(chn_id, in_size, input_yuv_file);
    if (ret != TD_SUCCESS) {
        sample_print("sample_aisr_process_pic fail.\n");
    }

    sample_aisr_stop(chn_id);

exit1:
    sample_comm_sys_exit();

    return ret;
}
