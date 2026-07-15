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
#define MAX_IMG_NAME_SIZE 512

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
    printf("Usage : %s <index> [vi/file]\n", prg_name);
    printf("index:\n");
    printf("\t 0: sample_svp_npu_acl_ovd.\n");
    printf("[vi/file]:\n");
    printf("\t 0: Board(VI->VPSS->SVP_NPU->VGS->VO); 1: Board(FILE->VDEC->VPSS->SVP_NPU->VGS->VO); \
        2: PC(Img->NPU->Postprocess run on cpu, see Note (4) in readme.)\n");
}

static td_s32 sample_svp_npu_case_with_two_args(td_char idx1, td_char idx2, const td_char *img_name)
{
    td_s32 ret = TD_SUCCESS;
    if (idx1 == '0' && !(idx2 >= '0' && idx2 <= '2')) {
        return TD_FAILURE;
    }
    switch (idx1) {
        case '0':
            sample_svp_npu_acl_ovd(idx2 - '0', img_name);
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
    if (argc > SAMPLE_SVP_NPU_ARG_NUM_FOUR || argc < SAMPLE_SVP_NPU_ARG_NUM_THREE) {
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
    if (argc >= SAMPLE_SVP_NPU_ARG_NUM_THREE) {
        size_t argv2_len = strlen(argv[SAMPLE_ARG_INDEX_TWO]);
        if (argv2_len != 1) {
            sample_svp_npu_usage(argv[0]);
            return TD_FAILURE;
        }
    }

    td_char *img_name = argv[SAMPLE_ARG_INDEX_THREE] == NULL? "" : argv[SAMPLE_ARG_INDEX_THREE];
    if (strlen(img_name) > MAX_IMG_NAME_SIZE) {
        printf("The image name cannot exceed 512 characters.\n");
        return TD_FAILURE;
    }
    ret = sample_svp_npu_case_with_two_args(*argv[1], *argv[SAMPLE_ARG_INDEX_TWO], img_name);
    if (ret != TD_SUCCESS) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    return 0;
}
