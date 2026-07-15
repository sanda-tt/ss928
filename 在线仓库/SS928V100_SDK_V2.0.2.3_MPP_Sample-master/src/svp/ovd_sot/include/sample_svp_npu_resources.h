/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: sample_svp_npu_process.c
 * Author: Hisilicon multimedia software group
 * Create: 2024-12-09
 */
#ifndef SAMPLE_SVP_NPU_RESOURCES_H
#define SAMPLE_SVP_NPU_RESOURCES_H

#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "svp_acl_rt.h"
#include "svp_acl.h"
#include "svp_acl_ext.h"
#include "sample_common_svp.h"
#include "sample_common_svp_npu.h"
#include "sample_common_svp_npu_model.h"

/* shared variates between files */
td_s32 *sample_svp_npu_get_device_id(td_void);

ot_sample_svp_rect_info *sample_svp_npu_get_svp_rect_info(td_void);

td_bool *sample_svp_npu_get_thread_stop(td_void);

sample_svp_npu_task_info *sample_svp_npu_get_task_info(int task_idx);

ot_sample_svp_media_cfg *sample_svp_npu_get_media_cfg(td_void);

sample_vdec_attr *sample_svp_npu_get_vdec_cfg(td_void);

vdec_thread_param *sample_svp_npu_get_vdec_param(td_void);

#endif