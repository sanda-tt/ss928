/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <signal.h>
#include <stdio.h>
#include "securec.h"
#include "ot_common_vb.h"
#include "sample_comm.h"
#include "sample_aisr_core.h"

volatile static sig_atomic_t g_sig_flag = 0;

static td_void sample_aisr_usage(const td_char *prg_name)
{
    printf("usage : %s input_yuv\n", prg_name);
    printf("        %s -h\n", prg_name);
}

static td_void sample_aisr_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        sample_print("catch int/term signal.\n");
        g_sig_flag = 1;
    }
}

static td_void sample_aisr_register_sig_handler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret;

    if (argc != 2) { /* 2:arg num */
        sample_aisr_usage(argv[0]);
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_aisr_usage(argv[0]);
        return TD_FAILURE;
    }

    if ((strlen(argv[1]) == 0)) {
        sample_aisr_usage(argv[0]);
        return TD_FAILURE;
    }

#ifndef __LITEOS__
    sample_aisr_register_sig_handler(sample_aisr_handle_sig);
#endif

    ret = sample_aisr_run_pic(argv[1]);
    if ((ret == TD_SUCCESS) && (g_sig_flag == 0)) {
        sample_print("\033[0;32msample aisr program exit normally!\033[0;39m\n");
    } else {
        sample_print("\033[0;31msample aisr program exit abnormally!\033[0;39m\n");
    }

#ifdef __LITEOS__
    return ret;
#else
    exit(ret);
#endif
}
