/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "ot_type.h"
#include "ot_defines.h"
#include "ot_common_uvc.h"
#include "camera.h"
#include "frame_cache.h"
#include "uvc_media.h"
#include "uvc.h"


#ifndef __LITEOS__
volatile sig_atomic_t g_need_quit_flag = 0;
sig_atomic_t sample_uvc_get_quit_flag(void)
{
    return g_need_quit_flag;
}
#endif

static void sample_uvc_usage(const char *s_prg_nm)
{
    printf("Usage1 : %s -h\n", s_prg_nm);
    printf("\t -h  --print help information\n");
    printf("Usage2 : %s [DEVICE_COUNT]\n", s_prg_nm);
    printf("\t DEVICE_COUNT: max uvc device count, should in [1, 2]\n");
}

#ifndef __LITEOS__
static void sample_uvc_handle_signal(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_need_quit_flag = 1;
    }

#if defined UVC_STILL_IMAGE && defined UVC_STILL_BUTTON_TEST
    if (signo == SIGUSR1) {
        button_pressed();
    }
#endif
}
#endif

static td_s32 sample_uvc_parse_args(int argc, char *argv[])
{
    int i = argc - 1;
    uint32_t dev_cnt = 1;

    while (i >= 0) {
        if (strcmp(argv[i], "-h") == 0) {
            return TD_FAILURE;
        }
        if (i == 1) { /* arg 1 for device count */
            if (strlen(argv[i]) != 1 || !isdigit(argv[i][0])) {
                printf("uvc device count invalid\n");
                return TD_FAILURE;
            }
            dev_cnt = strtoul(argv[i], TD_NULL, 10); /* 10 base */
            if (dev_cnt == 0 || dev_cnt > OT_UVC_MAX_CHN_NUM) {
                printf("uvc device count invalid\n");
                return TD_FAILURE;
            }
        }
        i--;
    }

    camera_set_uvc_device_cnt(dev_cnt);
    printf("UVC device count %u\n", camera_get_uvc_device_cnt());

    return TD_SUCCESS;
}

int main(int argc, char *argv[])
{
    int ret;

    ret = sample_uvc_parse_args(argc, argv);
    if (ret != TD_SUCCESS) {
        sample_uvc_usage(argv[0]);
        return 0;
    }

#ifndef __LITEOS__
    struct sigaction act = {0};
    act.sa_handler = sample_uvc_handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
#if defined UVC_STILL_IMAGE && defined UVC_STILL_BUTTON_TEST
    sigaction(SIGUSR1, &act, NULL);
#endif
#endif

    if (create_uvc_cache(camera_get_uvc_device_cnt()) != 0) {
        goto err;
    }

    if (get_g_uac_val() == 1) {
        if (create_uac_cache() != 0) {
            goto err;
        }
    }

    if (get_ot_camera()->init() != 0 || get_ot_camera()->open() != 0 || get_ot_camera()->run() != 0) {
        goto err;
    }

    printf("UVC sample exit!\n");

err:
    get_ot_camera()->close();
    destroy_uvc_cache();
    if (get_g_uac_val() == 1) {
        destroy_uac_cache();
    }

    printf("exit app normally\n");
    return 0;
}
