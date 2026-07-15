/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: sample_svp_npu_main.c
 * Author: Hisilicon multimedia software group
 * Create: 2024-12-09
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

#include "securec.h"
#include "sample_svp_npu_process.h"

#define SAMPLE_SVP_NPU_ARG_NUM_TWO 2
#define SAMPLE_SVP_NPU_ARG_NUM_THREE 3
#define SAMPLE_SVP_NPU_ARG_NUM_FOUR 4
#define SAMPLE_ARG_INDEX_TWO 2
#define SAMPLE_ARG_INDEX_THREE 3
#define SAMPLE_SVP_NPU_CMP_STR_NUM 2
#define SAMPLE_SVP_NPU_ARG_STR_LEN 2
#define SAMPLE_SVP_NPU_ARG_IDX_BASE 10

static char **g_npu_cmd_argv = TD_NULL;

/*
 * function : to process abnormal case
 */
static td_void sample_svp_npu_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        sample_svp_npu_acl_handle_sig();
    }
}

/*
 * function : show usage
 */
static td_void sample_svp_npu_usage(const td_char *prg_name)
{
    printf("Usage : %s <index> <vi/file> [sns_type]\n", prg_name);
    printf("index:\n");
    printf("\t 0: sample_svp_npu_acl_ovd.\n");
    printf("\t 1: sample_svp_npu_acl_ovd_sot.\n");
    printf("vi/file:\n");
    printf("\t 0: Board(VI->VPSS->SVP_NPU->VGS->VO);\n");
    printf("\t 1: Board(FILE->VDEC->VPSS->SVP_NPU->VGS->VO);\n");
    printf("sns_type: (optional)\n");
    printf("\t  NOTE: If sns_type is specified, sensor0~sensor3 will ALL use this type.\n");
    printf("\t        If not specified, sensor0~sensor3 use default values from Makefile.\n");
    printf("\t  2) OV_OS08A20_MIPI_8M_30FPS_12BIT\n");
    printf("\t  4) OV_OS04A10_MIPI_4M_30FPS_12BIT\n");
    printf("\t  8) SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT\n");
    printf("\t  12) SC450AI_MIPI_4M_30FPS_10BIT\n");
}

static td_s32 sample_svp_npu_case_with_two_args(td_char idx1, td_char idx2)
{
    td_s32 ret = TD_SUCCESS;
    if (!(idx1 >= '0' && idx1 <= '1') || !(idx2 >= '0' && idx2 <= '1')) {
        return TD_FAILURE;
    }
    switch (idx1) {
        case '0':
            sample_svp_npu_acl_ovd(idx2 - '0');
            break;
        case '1':
            sample_svp_npu_acl_ovd_sot(idx2 - '0');
            break;
        default:
            ret = TD_FAILURE;
            break;
    }
    return ret;
}

/*
 * function : svp npu sample
 */
int main(int argc, char *argv[])
{
    td_s32 ret;
    struct sigaction sa;
    if (argc < SAMPLE_SVP_NPU_ARG_NUM_THREE || argc > SAMPLE_SVP_NPU_ARG_NUM_FOUR) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", SAMPLE_SVP_NPU_CMP_STR_NUM)) {
        sample_svp_npu_usage(argv[0]);
        return TD_SUCCESS;
    }

    g_npu_cmd_argv = argv;
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_svp_npu_handle_sig;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    size_t argv1_len = strlen(argv[1]);
    if (argv1_len != 1) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }
    size_t argv2_len = strlen(argv[SAMPLE_ARG_INDEX_TWO]);
        if (argv2_len != 1) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    if (argc == SAMPLE_SVP_NPU_ARG_NUM_FOUR) {
        sample_sns_type sns_type;
        sns_type = (sample_sns_type)atoi(argv[3]);
        if (sns_type != 2 && sns_type != 4 && sns_type != 8 && sns_type != 12) {
            sample_svp_npu_usage(argv[0]);
            return TD_FAILURE;
        }
        set_sns_type(sns_type);
    }

    ret = sample_svp_npu_case_with_two_args(*argv[1], *argv[SAMPLE_ARG_INDEX_TWO]);
    if (ret != TD_SUCCESS) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    return 0;
}
