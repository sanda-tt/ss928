/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_COMMON_AISR_H
#define OT_COMMON_AISR_H

#include "ot_common_aiv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    ot_aiv_model model;
} ot_aisr_model;

typedef struct {
    td_bool enable;
    td_u32 input_depth;
    td_u32 output_depth;
} ot_aisr_attr;

typedef struct {
    ot_size size;
} ot_aisr_cfg;

typedef struct {
    td_bool enable;
} ot_aisr_status;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
