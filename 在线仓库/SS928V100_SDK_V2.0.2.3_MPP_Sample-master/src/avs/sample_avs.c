/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "ss_mpi_avs.h"
#include "sample_comm.h"
#include "ss_mpi_avs_lut_generate.h"
#include "ss_mpi_avs_pos_query.h"
#include "ss_mpi_avs_conversion.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static td_phys_addr_t  g_avs_lut_addr = 0;
static td_phys_addr_t  g_avs_mask_addr = 0;
static td_phys_addr_t  g_avs_convert_in_addr = 0;
static td_phys_addr_t  g_avs_convert_out_addr = 0;
static td_u64 g_mesh_addr = 0;

static td_char g_cal_file[OT_AVS_CALIBRATION_FILE_LEN];

#define LUT_NAME_MAX_LEN 64
#define FILE_MAX_LEN 128

static volatile sig_atomic_t g_avs_sig_flag = 0;

typedef struct {
    ot_size dst_size;  /* the output stitch image size */
    ot_size cell_size; /* the cell size of each block in mesh lut */
    ot_size mesh_size; /* the mesh vertices number */
    td_bool mesh_normalized; /* mesh is normalized or not (prevent reentry) */
    td_u32 cam_num;
} avs_convert_config;

static td_u32 sample_avs_get_file_len(const td_char *file)
{
    FILE *fd;
    td_u32 file_len;
    td_s32 ret;

    fd = fopen(file, "rb");
    if (fd != NULL) {
        ret = fseek(fd, 0L, SEEK_END);
        if (ret != 0) {
            sample_print("fseek err!\n");
            fclose(fd);
            return 0;
        }

        file_len = ftell(fd);

        ret = fseek(fd, 0L, SEEK_SET);
        if (ret != 0) {
            sample_print("fseek err!\n");
            fclose(fd);
            return 0;
        }

        fclose(fd);
    } else {
        sample_print("open file %s fail!\n", file);
        file_len = 0;
    }

    return file_len;
}

static td_s32 sample_avs_load_file(const td_char* file, td_char *addr, td_u32 size)
{
    FILE *fd;
    td_u32 read_bytes;

    fd = fopen(file, "rb");
    if (fd != NULL) {
        read_bytes = fread(addr, size, 1, fd);
        if (read_bytes != 1) {
            sample_print("read file error!\n");
            fclose(fd);
            return TD_FAILURE;
        }
        fclose(fd);
    } else {
        sample_print("Error: Open file of %s failed!\n", file);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_avs_lut_mask_cfg(ot_avs_lut_mask *lut_mask, td_u32 width, td_u32 height)
{
    lut_mask->input_yuv_mask_flag = TD_FALSE;
    lut_mask->same_mask_flag      = TD_FALSE;
    lut_mask->mask_define[0].mask_shape      = OT_AVS_MASK_SHAPE_RECT;
    lut_mask->mask_define[0].offset_h        = 0;
    lut_mask->mask_define[0].offset_v        = 0;
    lut_mask->mask_define[0].half_major_axis = width / 2;   /* half 2 */
    lut_mask->mask_define[0].half_minor_axis = height / 2;  /* half 2 */

    lut_mask->mask_define[1].mask_shape      = OT_AVS_MASK_SHAPE_ELLIPSE;
    lut_mask->mask_define[1].offset_h        = 500; /* 500 */
    lut_mask->mask_define[1].offset_v        = 500; /* 500 */
    lut_mask->mask_define[1].half_major_axis = width / 2; /* half 2 */
    lut_mask->mask_define[1].half_minor_axis = height / 2; /* half 2 */
}

static td_void sample_avs_lut_input_cfg(ot_avs_lut_generate_input *lut_input, ot_avs_lut_mask *lut_mask)
{
    ot_avs_fine_tuning   fine_tuning_cfg;

    fine_tuning_cfg.fine_tuning_en = TD_TRUE;

    fine_tuning_cfg.adjust[0].adjust_en   = TD_FALSE;
    fine_tuning_cfg.adjust[0].pitch    = 0;
    fine_tuning_cfg.adjust[0].yaw      = 0 * 100;  /* 100 */
    fine_tuning_cfg.adjust[0].roll     = -0 * 100; /* 100 */
    fine_tuning_cfg.adjust[0].offset_x  = 0;
    fine_tuning_cfg.adjust[0].offset_y  = 0;

    fine_tuning_cfg.adjust[1].adjust_en  = TD_TRUE;
    fine_tuning_cfg.adjust[1].pitch   = -10 * 100; /* 100 */
    fine_tuning_cfg.adjust[1].yaw     = 0;
    fine_tuning_cfg.adjust[1].roll    = 0;
    fine_tuning_cfg.adjust[1].offset_x = 0 * 100;  /* 100 */
    fine_tuning_cfg.adjust[1].offset_y = -0 * 100; /* 100 */

    lut_input->fine_tuning_cfg                = fine_tuning_cfg;
    lut_input->fine_tuning_cfg.fine_tuning_en = TD_FALSE;
    lut_input->lut_accuracy                   = OT_AVS_LUT_ACCURACY_HIGH;
    lut_input->type                           = OT_AVS_TYPE_AVSP;
    lut_input->stitch_distance                = 2.0; /* 2.0 */
    lut_input->mask                           = lut_mask;
    lut_input->file_input_addr                = (td_u64)(td_uintptr_t)g_cal_file;
}

static td_void sample_avs_lut_write(td_u32 length, td_u64 lut_output[OT_AVS_MAX_CAMERA_NUM], td_u32 max_len)
{
    td_s32 i, ret;
    td_char out_lut[2][FILE_MAX_LEN]; /* 2 */
    FILE *lut_file = NULL;

    for (i = 0; i < (td_s32)length; i++) {
        if (snprintf_s(out_lut[i], FILE_MAX_LEN, FILE_MAX_LEN - 1, "./2fisheye_3000x3000_mesh_%d.bin", i) == -1) {
            sample_print("set file name fail!\n");
            return;
        }
        lut_file = fopen(out_lut[i], "wb");
        if (lut_file == NULL) {
            sample_print("fopen err!\n");
            return;
        }
        ret = fwrite((td_char*)(td_uintptr_t)lut_output[i], OT_AVS_LUT_SIZE, 1, lut_file);
        if (ret < 0) {
            sample_print("fwrite err %d\n", ret);
            fclose(lut_file);
            return;
        }
        fclose(lut_file);
    }
}

static td_s32 sample_avs_lut_free()
{
    td_s32 ret = TD_SUCCESS;
    if (g_avs_lut_addr) {
        ret = ss_mpi_sys_mmz_free(g_avs_lut_addr, NULL);
        if (ret != TD_SUCCESS) {
            sample_print("mmz_free fail!\n");
        }
        g_avs_lut_addr = 0;
    }
    return ret;
}

static td_s32 sample_avs_mask_free()
{
    td_s32 ret = TD_SUCCESS;
    if (g_avs_mask_addr) {
        ret = ss_mpi_sys_mmz_free(g_avs_mask_addr, NULL);
        if (ret != TD_SUCCESS) {
            sample_print("mmz_free fail!\n");
        }
        g_avs_mask_addr = 0;
    }
    return ret;
}

static td_s32 sample_avs_generating_lut(void)
{
    const td_u32 camera_num = 2; /* 2 */
    const td_u32 width = 3000;   /* 3000 */
    const td_u32 height = 3000;  /* 3000 */
    td_u64  lut_output[OT_AVS_MAX_CAMERA_NUM];

    ot_avs_status status;
    td_void *mask_addr = NULL;
    td_void *lut_addr = NULL;
    td_s32 i, cal_size, ret;

    ot_avs_lut_generate_input lut_input;
    ot_avs_lut_mask lut_mask;

    cal_size = sample_avs_get_file_len("./data/2fisheye_3000x3000.cal");
    if (cal_size == 0) {
        sample_print("Open cal file fail!\n");
        return TD_FAILURE;
    }

    ret = sample_avs_load_file("./data/2fisheye_3000x3000.cal", g_cal_file, cal_size);
    check_return(ret, "Load cal file fail!");

    sample_avs_lut_mask_cfg(&lut_mask, width, height);

    ret = ss_mpi_sys_mmz_alloc(&(g_avs_mask_addr), &(mask_addr), "avs_mask", NULL, sizeof(td_u16) * width * height);
    if (ret != TD_SUCCESS) {
        sample_print("alloc mask buf fail with %#x!\n", ret);
        goto exit;
    }

    lut_mask.mask_addr[0] = (td_u64)(td_uintptr_t)mask_addr;
    ret = ss_mpi_sys_mmz_alloc(&(g_avs_lut_addr), &(lut_addr), "avs_lut", NULL, OT_AVS_LUT_SIZE * camera_num);
    if (ret != TD_SUCCESS) {
        sample_print("alloc lut buf fail with %#x!\n", ret);
        goto exit1;
    }

    for (i = 0; i < (td_s32)camera_num; i++) {
        lut_output[i] = (td_u64)(td_uintptr_t)lut_addr + i * OT_AVS_LUT_SIZE;
    }

    sample_avs_lut_input_cfg(&lut_input, &lut_mask);

    status = ss_mpi_avs_lut_generate(&lut_input, lut_output);
    if (status != OT_AVS_STATUS_OK) {
        sample_print("Generate lut error!\n");
        goto exit2;
    }

    sample_avs_lut_write(camera_num, lut_output, OT_AVS_MAX_CAMERA_NUM * sizeof(td_u64));

exit2:
    ret = sample_avs_lut_free();

exit1:
    ret = sample_avs_mask_free();

exit:
    return ret;
}

static td_s32 sample_avs_malloc(ot_avs_pos_cfg *avs_cfg, td_u64 mesh_addr[], td_u32 max_len, td_u32 len)
{
    td_u32 i;
    td_u32 mesh_size, mesh_point_x, mesh_point_y;
    if (avs_cfg->mesh_mode == OT_AVS_DST_QUERY_SRC) {
        mesh_point_x = (avs_cfg->dst_size.width - 1) / avs_cfg->window_size + 2;  /* 2 */
        mesh_point_y = (avs_cfg->dst_size.height - 1) / avs_cfg->window_size + 2; /* 2 */
    } else {
        mesh_point_x = (avs_cfg->src_size.width - 1) / avs_cfg->window_size + 2;  /* 2 */
        mesh_point_y = (avs_cfg->src_size.height - 1) / avs_cfg->window_size + 2; /* 2 */
    }
    mesh_size = sizeof(td_s16) * mesh_point_x * mesh_point_y * 2; /* 2 */

    g_mesh_addr = (td_ulong)malloc(mesh_size * avs_cfg->camera_num);
    if (g_mesh_addr == 0) {
        sample_print("malloc error!\n");
        return TD_FAILURE;
    }
    for (i = 0; i < len; i++) {
        mesh_addr[i] = g_mesh_addr + mesh_size * i;
    }
    return TD_SUCCESS;
}

static td_void sample_avs_malloc_free()
{
    if (g_mesh_addr) {
        free((td_void *)(td_ulong)g_mesh_addr);
        g_mesh_addr = 0;
    }
}

static td_s32 sample_avs_read_lut_data(ot_avs_pos_cfg *avs_cfg)
{
    td_s32 ret, i;
    td_void *lut_start_addr = NULL;
    td_void *lut_addr[OT_AVS_MAX_INPUT_NUM];
    td_char  lut_path[FILE_MAX_LEN];

    ret = ss_mpi_sys_mmz_alloc(&(g_avs_lut_addr), &(lut_start_addr), "avs_lut",
        NULL, OT_AVS_LUT_SIZE * avs_cfg->camera_num);
    if (ret != TD_SUCCESS) {
        sample_print("alloc lut buf fail with %#x!\n", ret);
        return TD_FAILURE;
    }

    for (i = 0; i < (td_s32)avs_cfg->camera_num; i++) {
        lut_addr[i] = (td_void *)((td_ulong)lut_start_addr + OT_AVS_LUT_SIZE * i);

        if (snprintf_s(lut_path, FILE_MAX_LEN, FILE_MAX_LEN - 1, "./data/avsp_mesh_out_%d.bin", i + 1) == -1) {
            sample_print("set file name fail!\n");
            return TD_FAILURE;
        }

        FILE    *lut_file = NULL;

        lut_file = fopen(lut_path, "rb");
        if (lut_file == NULL) {
            sample_print("open mesh file (%s) fail!\n", lut_path);
            return TD_FAILURE;
        }

        ret = fread((td_char *)lut_addr[i], OT_AVS_LUT_SIZE, 1, lut_file);
        if (ret != 1) {
            sample_print("Error reading file %s\n", lut_path);
            ret = TD_FAILURE;
        }

        avs_cfg->lut_addr[i] = (td_ulong)lut_addr[i];

        fclose(lut_file);
    }
    return TD_SUCCESS;
}

static td_void sample_avs_lut_data_free()
{
    td_s32 ret;
    if (g_avs_lut_addr) {
        ret = ss_mpi_sys_mmz_free(g_avs_lut_addr, NULL);
        if (ret != TD_SUCCESS) {
            sample_print("mmz_free fail!\n");
        }
        g_avs_lut_addr = 0;
    }
}

static td_void sample_avs_get_mesh_cfg(td_u32 *mesh_size, td_u32 *mesh_point_x, td_u32 *mesh_point_y,
    ot_avs_pos_cfg *avs_cfg)
{
    if (avs_cfg->mesh_mode == OT_AVS_DST_QUERY_SRC) {
        *mesh_point_x = (avs_cfg->dst_size.width - 1) / avs_cfg->window_size + 2;  /* 2 winsize */
        *mesh_point_y = (avs_cfg->dst_size.height - 1) / avs_cfg->window_size + 2; /* 2 winsize */
    } else {
        *mesh_point_x = (avs_cfg->src_size.width - 1) / avs_cfg->window_size + 2; /* 2 winsize */
        *mesh_point_y = (avs_cfg->src_size.height - 1) / avs_cfg->window_size + 2; /* 2 winsize */
    }
    *mesh_size = sizeof(td_s16) * (*mesh_point_x) * (*mesh_point_y) * 2; /* 2 winsize */
}

static td_s32 sample_avs_save_mesh_file(ot_avs_pos_cfg *avs_cfg, const td_char *file_prefix,
    td_u64 mesh_addr[OT_AVS_MAX_INPUT_NUM], td_u32 max_len, td_u32 len)
{
    td_char file_path1[FILE_MAX_LEN + 1], file_path2[FILE_MAX_LEN + 1], file_index[FILE_MAX_LEN];
    td_u32 mesh_size, mesh_point_x, mesh_point_y;

    sample_avs_get_mesh_cfg(&mesh_size, &mesh_point_x, &mesh_point_y, avs_cfg);

    for (td_s32 i = 0; i < (td_s32)len; i++) {
        if (snprintf_s(file_index, FILE_MAX_LEN, FILE_MAX_LEN - 1, "%d.csv", i) == -1) {
            sample_print("set file name fail!\n");
            return TD_FAILURE;
        }
        strncpy_s(file_path1, sizeof(file_path1), file_prefix, FILE_MAX_LEN);
        if (strcat_s(file_path1, sizeof(file_path1), file_index) != 0) {
            return TD_FAILURE;
        }

        FILE *fp = fopen(file_path1, "wb");
        if (fp == NULL) {
            sample_print("Can not open file.\n");
            return TD_FAILURE;
        }

        td_s16 *addr = (td_s16 *)(td_ulong)mesh_addr[i];
        for (td_s32 n = 0; n < (td_s32)mesh_point_y; n++) {
            for (td_s32 m = 0; m < (td_s32)mesh_point_x; m++) {
                td_s16 x_index = addr[n * mesh_point_x * 2 + m * 2]; /* 2 */
                td_s16 y_index = addr[n * mesh_point_x * 2 + m * 2 + 1]; /* 2 */

                fprintf(fp, "%d %d,", x_index, y_index);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);

        if (snprintf_s(file_index, FILE_MAX_LEN, FILE_MAX_LEN - 1, "%d.bin", i) == -1) {
            sample_print("set output file name fail!\n");
            return TD_FAILURE;
        }
        strncpy_s(file_path2, sizeof(file_path2), file_prefix, FILE_MAX_LEN);
        if (strcat_s(file_path2, sizeof(file_path2), file_index) != 0) {
            return TD_FAILURE;
        }
        fp = fopen(file_path2, "wb");
        if (fp == NULL) {
            sample_print("Can not open file.\n");
            return TD_FAILURE;
        }

        if (fwrite((td_s16*)(td_ulong)mesh_addr[i], mesh_size, 1, fp) != 1) {
            sample_print("write file fail!\n");
        }
        fclose(fp);
    }
    return TD_SUCCESS;
}

static td_s32 sample_avs_dst_to_src(ot_avs_pos_cfg *avs_cfg)
{
    td_s32 ret, i;
    td_u64 mesh_addr[OT_AVS_MAX_INPUT_NUM];
    ot_size dst_size;
    ot_point src_point, dst_point;
    const td_u32 length = 4;  /* 4 length */

    avs_cfg->projection_mode = OT_AVS_PROJECTION_EQUIRECTANGULAR;
    avs_cfg->src_size.width  = 3840;  /* 3840 */
    avs_cfg->src_size.height = 2160;  /* 2160 */
    avs_cfg->dst_size.width  = 7680;  /* 7680 */
    avs_cfg->dst_size.height = 4320;  /* 4320 */
    avs_cfg->center.x  = avs_cfg->dst_size.width / 2;  /* 2 */
    avs_cfg->center.y  = avs_cfg->dst_size.height / 2; /* 2 */
    avs_cfg->fov.fov_x = 18000;  /* 18000 */
    avs_cfg->fov.fov_y = 8000;   /* 8000 */
    avs_cfg->ori_rotation.yaw   = 0;
    avs_cfg->ori_rotation.pitch = 6500;  /* 6500 */
    avs_cfg->ori_rotation.roll  = -9000; /* -9000 */
    avs_cfg->rotation.yaw      = 0;
    avs_cfg->rotation.pitch    = 0;
    avs_cfg->rotation.roll     = 0;
    avs_cfg->mesh_mode         = OT_AVS_DST_QUERY_SRC;
    avs_cfg->window_size       = 128;  /* 128 */
    avs_cfg->lut_accuracy      = OT_AVS_LUT_ACCURACY_HIGH;

    ret = sample_avs_malloc(avs_cfg, mesh_addr, OT_AVS_MAX_INPUT_NUM, length);
    check_return(ret, "sample_avs_dst_to_src");

    ret = ss_mpi_avs_pos_mesh_generate(avs_cfg, mesh_addr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_avs_pos_mesh_generate failed, error:%d \n", ret);
        sample_avs_malloc_free();
        return TD_FAILURE;
    }

    const td_char *file_prefix = "./data/DST_QUERY_SRC_win128_4x3840x2160_7680x4320_";
    ret = sample_avs_save_mesh_file(avs_cfg, file_prefix, mesh_addr, OT_AVS_MAX_INPUT_NUM * sizeof(td_u64), length);
    if (ret != TD_SUCCESS) {
        sample_avs_malloc_free();
        return TD_FAILURE;
    }

    dst_size.width = avs_cfg->dst_size.width;
    dst_size.height = avs_cfg->dst_size.height;
    dst_point.x = 1000;  /* 1000 x */
    dst_point.y = 1000;  /* 1000 y */
    for (i = 0; i < (td_s32)length; i++) {
        ret = ss_mpi_avs_pos_query_dst_to_src(&dst_size, avs_cfg->window_size, mesh_addr[i], &dst_point, &src_point);
        printf("dst_to_src In: (%d,%d) ->%d:(%d,%d) \n", dst_point.x, dst_point.y, i, src_point.x, src_point.y);
    }
    sample_avs_malloc_free();
    return TD_SUCCESS;
}

static td_s32 sample_avs_src_to_dst(ot_avs_pos_cfg *avs_cfg)
{
    td_s32 ret, i;
    td_u64 mesh_addr[OT_AVS_MAX_INPUT_NUM];
    ot_size src_size;
    ot_point src_point, dst_point;
    const td_u32 length = 4;  /* 4length */

    avs_cfg->projection_mode = OT_AVS_PROJECTION_EQUIRECTANGULAR;
    avs_cfg->src_size.width  = 3840;  /* 3840 */
    avs_cfg->src_size.height = 2160;  /* 2160 */
    avs_cfg->dst_size.width  = 4000;  /* 4000 */
    avs_cfg->dst_size.height = 2000;  /* 2000 */
    avs_cfg->center.x  = avs_cfg->dst_size.width / 2; /* 2 */
    avs_cfg->center.y  = avs_cfg->dst_size.height / 2; /* 2 */
    avs_cfg->fov.fov_x = 16000;  /* 16000 */
    avs_cfg->fov.fov_y = 8000;   /* 8000 */
    avs_cfg->ori_rotation.yaw   = 0;
    avs_cfg->ori_rotation.pitch = 5000;  /* 5000 */
    avs_cfg->ori_rotation.roll  = -9000; /* -9000 */
    avs_cfg->rotation.yaw      = 0;
    avs_cfg->rotation.pitch    = 0;
    avs_cfg->rotation.roll     = 0;
    avs_cfg->mesh_mode         = OT_AVS_SRC_QUERY_DST;
    avs_cfg->window_size       = 128;  /* 128 */
    avs_cfg->lut_accuracy      = OT_AVS_LUT_ACCURACY_HIGH;

    ret = sample_avs_malloc(avs_cfg, mesh_addr, OT_AVS_MAX_INPUT_NUM * sizeof(td_u64), length);
    check_return(ret, "sample_avs_src_to_dst");

    ret = ss_mpi_avs_pos_mesh_generate(avs_cfg, mesh_addr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_avs_pos_mesh_generate failed, error:%d \n", ret);
        sample_avs_malloc_free();
        return TD_FAILURE;
    }

    const td_char *file_prefix = "./data/SRC_QUERY_DST_win128_4x3840x2160_4000x2000_";
    ret = sample_avs_save_mesh_file(avs_cfg, file_prefix, mesh_addr, OT_AVS_MAX_INPUT_NUM * sizeof(td_u64), length);
    if (ret != TD_SUCCESS) {
        sample_avs_malloc_free();
        return TD_FAILURE;
    }
    src_size.width = avs_cfg->src_size.width;
    src_size.height = avs_cfg->src_size.height;
    src_point.x = 1000;  /* 1000 x */
    src_point.y = 1000;  /* 1000 y */
    for (i = 0; i < (td_s32)length; i++) {
        ret = ss_mpi_avs_pos_query_src_to_dst(&src_size, avs_cfg->window_size, mesh_addr[i], &src_point, &dst_point);
        printf("src_to_dst In: %d:(%d,%d) -> (%d,%d) \n", i, src_point.x, src_point.y, dst_point.x, dst_point.y);
    }
    sample_avs_malloc_free();
    return TD_SUCCESS;
}

static td_s32 sample_avs_pos_query(void)
{
    td_s32 ret;
    ot_avs_pos_cfg avs_cfg;

    avs_cfg.camera_num = 4; /* 4 */
    ret = sample_avs_read_lut_data(&avs_cfg);
    if (ret != TD_SUCCESS) {
        sample_avs_lut_data_free();
        return TD_FAILURE;
    }

    /* 1. pos_query_dst_to_src */
    ret = sample_avs_dst_to_src(&avs_cfg);
    if (ret != TD_SUCCESS) {
        sample_avs_lut_data_free();
        return TD_FAILURE;
    }

    /* 2. pos_query_src_to_dst */
    ret = sample_avs_src_to_dst(&avs_cfg);
    if (ret != TD_SUCCESS) {
        sample_avs_lut_data_free();
        return TD_FAILURE;
    }

    sample_avs_lut_data_free();
    return TD_SUCCESS;
}

static td_void sample_avs_convert_config_file_path(td_char input_file_path[4][FILE_MAX_LEN],
    td_char avsp_mesh_path[4][FILE_MAX_LEN])
{
    /* input and output file path */
    strncpy_s(input_file_path[0], sizeof(input_file_path[0]),
        "./data/avs_convert/lut_cell10x8_dst5108x1520_head_0.bin", FILE_MAX_LEN);
    strncpy_s(input_file_path[1], sizeof(input_file_path[1]),
        "./data/avs_convert/lut_cell10x8_dst5108x1520_head_1.bin", FILE_MAX_LEN);
    strncpy_s(avsp_mesh_path[0], sizeof(avsp_mesh_path[0]), "./data/avs_convert/lut_avsp_0.bin", FILE_MAX_LEN);
    strncpy_s(avsp_mesh_path[1], sizeof(avsp_mesh_path[1]), "./data/avs_convert/lut_avsp_1.bin", FILE_MAX_LEN);
}

static td_s32 sample_avs_convert_read_lut_header(avs_convert_config *cfg, const char *file_path, FILE *lut_file)
{
    td_s32 ret;

    /* read lut header configuration */
    ret = fread(&cfg->mesh_size.width, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }

    ret = fread(&cfg->mesh_size.height, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }

    ret = fread(&cfg->cell_size.width, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }

    ret = fread(&cfg->cell_size.height, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }

    ret = fread(&cfg->dst_size.width, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }

    ret = fread(&cfg->dst_size.height, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }

    ret = fread(&cfg->mesh_normalized, sizeof(td_u32), 1, lut_file);
    if (ret != 1) {
        sample_print("Error reading file %s\n", file_path);
        ret = TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void sample_avs_convert_free_input()
{
    td_s32 ret;

    if (g_avs_convert_in_addr) {
        ret = ss_mpi_sys_mmz_free(g_avs_convert_in_addr, NULL);
        if (ret != TD_SUCCESS) {
            sample_print("mmz_free fail!\n");
        }
        g_avs_convert_in_addr = 0;
    }
}

static td_void sample_avs_convert_free_output()
{
    td_s32 ret;
    if (g_avs_convert_out_addr) {
        ret = ss_mpi_sys_mmz_free(g_avs_convert_out_addr, NULL);
        if (ret != TD_SUCCESS) {
            sample_print("mmz_free fail!\n");
        }
        g_avs_convert_out_addr = 0;
    }
}

static td_s32 sample_avs_convert_cam(td_s32 cam, avs_convert_config *cfg,
    td_char input_file_path[4][FILE_MAX_LEN], td_char avsp_mesh_path[4][FILE_MAX_LEN])
{
    td_s32 ret;
    td_void *mesh_in_addr = TD_NULL;
    td_void *mesh_out_addr = TD_NULL;

    FILE *lut_file = fopen(input_file_path[cam], "rb");
    check_null_ptr_return(lut_file);

    if (sample_avs_convert_read_lut_header(cfg, input_file_path[cam], lut_file) != TD_SUCCESS) {
        sample_print("convert read lut header fail with %#x!\n", ret);
        goto end0;
    }

    const td_u32 lut_size = 6 * cfg->mesh_size.width * cfg->mesh_size.height + AVS_FILE_HEADER_SIZE; /* 6 byte(x,y,a) */
    printf("lut_size = %d \n", lut_size);

    /* malloc memory for mesh_in_addr */
    ret = ss_mpi_sys_mmz_alloc(&(g_avs_convert_in_addr), &(mesh_in_addr), "lut_input", NULL, lut_size);
    if (ret != TD_SUCCESS) {
        sample_print("alloc lut_input buf fail with %#x!\n", ret);
        goto end0;
    }

    /* malloc memory for mesh_out_addr */
    ret = ss_mpi_sys_mmz_alloc(&(g_avs_convert_out_addr), &(mesh_out_addr), "lut_output", NULL, AVS_LUT_SIZE);
    if (ret != TD_SUCCESS) {
        sample_print("alloc lut_output buf fail with %#x!\n", ret);
        goto end1;
    }

    (td_void)memset_s(mesh_out_addr, AVS_LUT_SIZE, 0, AVS_LUT_SIZE);

    (td_void)fseek(lut_file, 0, SEEK_SET); /* set lut_file to its head address */

    if (fread(mesh_in_addr, lut_size, 1, lut_file) != 1) {
        sample_print("Error reading lut file!\n");
    }

    ret = ss_mpi_avs_conversion((td_u64)(td_uintptr_t)mesh_in_addr, (td_u64)(td_uintptr_t)mesh_out_addr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_avs_conversion fail with %#x!\n", ret);
        goto end2;
    }

    FILE *lut_out_file = fopen(avsp_mesh_path[cam], "wb");
    if (lut_out_file == TD_NULL) {
        sample_print("open fail with %#x!\n", ret);
        goto end2;
    }

    if (fwrite((td_char *)(td_uintptr_t)mesh_out_addr, OT_AVS_LUT_SIZE, 1, lut_out_file) < 0) {
        sample_print("fwrite err!\n");
    }

    (td_void)fclose(lut_out_file);

end2:
    sample_avs_convert_free_output();
end1:
    sample_avs_convert_free_input();
end0:
    (td_void)fclose(lut_file);

    return ret;
}

static td_s32 sample_avs_convert(td_void)
{
    td_s32 ret, cam;
    td_char input_file_path[4][FILE_MAX_LEN]; /* 4: input lut path */
    td_char avsp_mesh_path[4][FILE_MAX_LEN];  /* 4: output avsp lut path */
    avs_convert_config cfg = {0};

    cfg.cam_num = 2; /* camera/lut number 2 */
    sample_avs_convert_config_file_path(input_file_path, avsp_mesh_path);

    for (cam = 0; cam < cfg.cam_num; cam++) {   /* camera/lut number 2 */
        ret = sample_avs_convert_cam(cam, &cfg, input_file_path, avsp_mesh_path);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_s32 sample_avs_get_rotation_matrix(void)
{
    const td_u32 camera_num = 2; /* 2 */
    td_s32 ret;
    ot_avs_status status;
    td_u32 cal_size, cam, i;
    td_u64 file_input_addr;
    td_double rotation_matrix[OT_AVS_MAX_CAMERA_NUM][OT_AVS_MATRIX_SIZE];

    cal_size = sample_avs_get_file_len("./data/2fisheye_3000x3000.cal");
    if (cal_size == 0) {
        sample_print("Open cal file fail!\n");
        return TD_FAILURE;
    }

    ret = sample_avs_load_file("./data/2fisheye_3000x3000.cal", g_cal_file, cal_size);
    check_return(ret, "Load cal file fail!");

    file_input_addr = (td_u64)(td_uintptr_t)g_cal_file;

    status = ss_mpi_avs_get_rotation_matrix(file_input_addr, rotation_matrix);
    if (status != OT_AVS_STATUS_OK) {
        sample_print("Get rotation matrix error!\n");
        return TD_FAILURE;
    }

    for (cam = 0; cam < camera_num; cam++) {
        printf("camera_%d rotation matrix:\n", cam);
        for (i = 0; i < 3; i++) { /* matrix dimension: 3 */
            printf("\t%f  %f  %f\n", rotation_matrix[cam][i * 3], rotation_matrix[cam][i * 3 + 1], /* matrix dim: 3 */
                rotation_matrix[cam][i * 3 + 2]); /* matrix dimension: 3; 2 for 3rd */
        }
    }

    return TD_SUCCESS;
}

#ifndef __LITEOS__
static void sample_avs_handle_signal(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_avs_sig_flag = 1;
    }
}
#endif

static void sample_avs_usage(const char *name)
{
    printf("usage : %s <index>\n", name);
    printf("index:\n");
    printf("\t 0) generate lut.\n");
    printf("\t 1) position query, dst->src & src->dst.\n");
    printf("\t 2) avs convert.\n");
    printf("\t 3) avs get rotation matrix.\n");

    return;
}

#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    td_s32 ret = TD_FAILURE;

    if (argc < 2) { /* 2 argc */
        sample_avs_usage(argv[0]);
        return TD_FAILURE;
    }

#ifndef __LITEOS__
    struct sigaction sa;
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_avs_handle_signal;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
#endif

    switch (*argv[1]) {
        case '0':
            ret = sample_avs_generating_lut();
            break;
        case '1':
            ret = sample_avs_pos_query();
            break;
        case '2':
            ret = sample_avs_convert();
            break;
        case '3':
            ret = sample_avs_get_rotation_matrix();
            break;
        default:
            sample_print("the index is invalid!\n");
            sample_avs_usage(argv[0]);
            return TD_FAILURE;
    }

    if (ret == TD_SUCCESS && g_avs_sig_flag == 0) {
        printf("\033[0;32mprogram exit normally!\033[0;39m\n");
    } else {
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

