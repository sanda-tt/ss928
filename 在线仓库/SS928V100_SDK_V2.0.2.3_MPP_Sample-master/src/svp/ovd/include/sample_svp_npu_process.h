/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: sample_svp_npu_process.h
 * Author: Hisilicon multimedia software group
 * Create: 2024-12-09
 */
#ifndef SAMPLE_SVP_NPU_PROCESS_H
#define SAMPLE_SVP_NPU_PROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "ot_type.h"
#include "sample_svp_npu_resources.h"

/* NMS_BOX_THR_OPT = 1000
* Generally, after the score threshold is used, only a few valid boxes are input to the NMS.
* In this example, 1000 valid boxes are set as a threshold. If the number of valid boxes is greater than 1000,
* the for loop is performed. If the number of valid boxes is less than 1000, the for loop is omitted.
*/
#define NMS_BOX_THR_OPT 1000 // the code is optimized to reduce the CPU usage.
#define NMS_BOX_MAX_NUM 30000 // Max box num to calc nms
#define TOP_K_LIMIT 300 // topk's k = 300
#define CLASS_NUM 80 // model fixed 80 class for Coco, if the model is n-class, change the value of this parameter
#define NAME_LEN 64

/* function : show the sample of sign handle */
td_void sample_svp_npu_acl_handle_sig(td_void);

/* function : show the sample of ovd */
td_void sample_svp_npu_acl_ovd(td_u32 is_file, const td_char *img_name);

/* function : show the sample of ovd to board */
td_void sample_svp_npu_acl_ovd_board(td_u32 is_file);

/* function : show the sample of ovd to pc */
td_void sample_svp_npu_acl_ovd_pc(const td_char *img_name);

/* function : read ovd config class file: class_lists.txt and text_config.txt */
td_s32 read_file(const td_char *src, td_char class[CLASS_NUM][NAME_LEN]);

/* function : process ovd second input */
td_s32 sample_common_svp_npu_get_ovd_second_input_data(const td_char *src, const sample_svp_npu_task_info *task);

/* function : ovd second input pre process */
td_s32 ovd_second_input_pre_process(const sample_svp_npu_task_info *task);
#endif
