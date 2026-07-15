/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_AISR_CORE_H
#define SAMPLE_AISR_CORE_H

#include "ot_type.h"

#define sample_aisr_check_exps_return(exps, ret, fmt, ...) \
    do {                                                   \
        if ((exps)) {                                      \
            sample_print(fmt, ##__VA_ARGS__);              \
            return ret;                                    \
        }                                                  \
    } while (0)

#define sample_aisr_check_exps_goto(exps, label, fmt, ...) \
    do {                                                   \
        if ((exps)) {                                      \
            sample_print(fmt, ##__VA_ARGS__);              \
            goto label;                                    \
        }                                                  \
    } while (0)

td_s32 sample_aisr_run_pic(const td_char *input_yuv_file);

#endif // SAMPLE_AISR_CORE_H
