/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "sample_common_svp.h"
#include "sample_npu_process.h"
#include "sample_common_ive.h"

#define OBJECT_SCORE_THRESHOLD 0.85f

static td_bool g_stop_signal = TD_FALSE;
static pthread_t g_yolo_thread;

static ot_sample_svp_switch g_yolo_switch = { TD_FALSE, TD_TRUE };
static sample_vi_cfg g_vi_config;

static ot_vb_blk vb_blk;

static uint64_t getms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

static td_s32 framecpy(ot_svp_dst_img *dstf, ot_video_frame_info *srcf)
{
    td_s32 ret = OT_ERR_IVE_NULL_PTR;
    ot_ive_handle handle;
    ot_svp_src_data src_data;
    ot_svp_dst_data dst_data;
    ot_ive_dma_ctrl ctrl = {OT_IVE_DMA_MODE_DIRECT_COPY, 0, 0, 0, 0};
    td_bool is_finish = TD_FALSE;
    td_bool is_block = TD_TRUE;
    // Copy Y
    int w = dstf->width;
    int h = dstf->height;
    src_data.phys_addr = srcf->video_frame.phys_addr[0];
    src_data.width = w;  // w;
    src_data.height = h; // h;
    src_data.stride = dstf->stride[0];

    dst_data.phys_addr = dstf->phys_addr[0];
    dst_data.width = w;  // w;
    dst_data.height = h; // h;
    dst_data.stride = dstf->stride[0];
    ret = ss_mpi_ive_dma(&handle, &src_data, &dst_data, &ctrl, TD_TRUE);
    if (ret != TD_SUCCESS) {

        return -1;
    }
    ret = ss_mpi_ive_query(handle, &is_finish, is_block);
    while (ret == OT_ERR_IVE_QUERY_TIMEOUT) {
        usleep(100);
        ret = ss_mpi_ive_query(handle, &is_finish, is_block);
    }
    // Copy UV
    src_data.phys_addr = srcf->video_frame.phys_addr[1];
    src_data.width = w;      // w;
    src_data.height = h / 2; // h;
    src_data.stride = dstf->stride[0];

    dst_data.phys_addr = dstf->phys_addr[1];
    dst_data.width = w;      // w;
    dst_data.height = h / 2; // h;
    dst_data.stride = dstf->stride[0];

    ret = ss_mpi_ive_dma(&handle, &src_data, &dst_data, &ctrl, TD_TRUE);
    if (ret != TD_SUCCESS) {
        return -1;
    }
    ret = ss_mpi_ive_query(handle, &is_finish, is_block);
    while (ret == OT_ERR_IVE_QUERY_TIMEOUT) {
        usleep(100);
        ret = ss_mpi_ive_query(handle, &is_finish, is_block);
    }
    return TD_SUCCESS;
}

static td_s32 CreateUsrFrame(ot_svp_img *imgAlgo, int w, int h)
{
    int vbsize = w * h * 3 / 2;
    vb_blk = ss_mpi_vb_get_blk(OT_VB_INVALID_POOL_ID, vbsize, TD_NULL);
    imgAlgo->phys_addr[0] = ss_mpi_vb_handle_to_phys_addr(vb_blk);
    imgAlgo->phys_addr[1] = imgAlgo->phys_addr[0] + w * h;
    imgAlgo->virt_addr[0] = (td_u8 *)ss_mpi_sys_mmap(imgAlgo->phys_addr[0], vbsize);
    imgAlgo->virt_addr[1] = imgAlgo->virt_addr[0] + w * h;
    imgAlgo->stride[0] = w;
    imgAlgo->stride[1] = w;
    imgAlgo->width = w;
    imgAlgo->height = h;
    imgAlgo->type = OT_SVP_IMG_TYPE_YUV420SP;
}

void yolov5objs_to_rect(const stYolov5Objs *pOut, ot_sample_svp_rect_info *rect)
{
    rect->num = 0;
    int max_num = (pOut->count < OT_SVP_RECT_NUM) ? pOut->count : OT_SVP_RECT_NUM;

    for (int i = 0; i < max_num; i++) {

        printf("\033[1;33m%d = %.5f at %d %d %d %d\033[0m\n",
        pOut->objs[i].label, pOut->objs[i].score,
        pOut->objs[i].x, pOut->objs[i].y,
        pOut->objs[i].w, pOut->objs[i].h);

        if(pOut->objs[i].score < OBJECT_SCORE_THRESHOLD) {
            continue;
        }

        int x = pOut->objs[i].x;
        int y = pOut->objs[i].y;
        int w = pOut->objs[i].w;
        int h = pOut->objs[i].h;

        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
        x &= ~1;
        y &= ~1;
        w &= ~1;
        h &= ~1;

        rect->rect[i].point[0].x = x;
        rect->rect[i].point[0].y = y;

        rect->rect[i].point[1].x = x + w;
        rect->rect[i].point[1].y = y;

        rect->rect[i].point[2].x = x + w;
        rect->rect[i].point[2].y = y + h;

        rect->rect[i].point[3].x = x;
        rect->rect[i].point[3].y = y + h;

        rect->num++;
    }
}

static td_void *sample_nnn_proc(td_void *args)
{
    td_s32 ret;
    ot_vpss_grp vpss_grp = 0;
    ot_vpss_chn vpss_chn = 1;
    td_s32 milli_sec = 20000;
    ot_video_frame_info frame_info;

    nnn_init();

    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    policy = SCHED_FIFO;
    param.sched_priority = sched_get_priority_max(policy);
    ret = pthread_setschedparam(pthread_self(), policy, &param);
    if (ret != 0) {
        printf("Failed to set thread scheduling: %s\n", strerror(ret));
    }

    stYolov5Objs *pOut = (stYolov5Objs *)malloc(sizeof(stYolov5Objs));
    ot_svp_img imgAlgo;
    CreateUsrFrame(&imgAlgo, 640, 640);

    while (g_stop_signal == TD_FALSE) {

        ret = ss_mpi_vpss_get_chn_frame(vpss_grp, vpss_chn, &frame_info, milli_sec);
        if (ret != TD_SUCCESS) {
            printf("ss_mpi_vpss_get_chn_frame err:0x%x\n", ret);
            continue;
        }
        printf("get frame sucess: wxh, %dx%d\n", frame_info.video_frame.width, frame_info.video_frame.height);

        int w = imgAlgo.width;
        int h = imgAlgo.height;
        int framelen = w * h * 3 / 2;
        void *pvit = ss_mpi_sys_mmap_cached(frame_info.video_frame.phys_addr[0], framelen);
        framecpy(&imgAlgo, &frame_info);

        nnn_execute(imgAlgo.virt_addr[0], framelen, pOut);

        /* Draw rect */
        ot_sample_svp_rect_info rect;
        yolov5objs_to_rect(pOut, &rect);
        ret = sample_common_svp_vgs_fill_rect(&frame_info, &rect, 0x0000FF00);
        if (ret != TD_SUCCESS) {
            printf("sample_svp_vgs_fill_rect err:0x%x\n", ret);
        }

        ss_mpi_sys_munmap(pvit, framelen);

        ret = ss_mpi_vo_send_frame(0, 0, &frame_info, milli_sec);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vo_send_frame failed, ret:0x%x\n", ret);
        }

        ret = ss_mpi_vpss_release_chn_frame(vpss_grp, vpss_chn, &frame_info);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vpss_release_chn_frame failed, ret:0x%x\n", ret);
        }
    }

    if (pOut) {
        free(pOut);
    }
    ss_mpi_sys_munmap(imgAlgo.virt_addr[0], imgAlgo.width * imgAlgo.height * 3 / 2);
    ss_mpi_vb_release_blk(vb_blk);

    return TD_NULL;
}

static td_s32 sample_yolo_pause(td_void)
{
    printf("---------------press Enter key to exit!---------------\n");

    (void)getchar();

    if (g_stop_signal == TD_TRUE) {
        if (g_yolo_thread != 0) {
            pthread_join(g_yolo_thread, TD_NULL);
            g_yolo_thread = 0;
        }

        sample_common_svp_stop_vi_vpss_venc_vo(&g_vi_config, &g_yolo_switch);
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_void sample_yolo_handle_sig(td_s32 singal)
{
    if (singal == SIGINT || singal == SIGTERM) {
        g_stop_signal = TD_TRUE;
    }
}

td_void sample_yolo(td_void)
{
    td_s32 ret;
    ot_size pic_size;
    ot_pic_size pic_type = PIC_640P;

    /*
     * step 1: start vi vpss venc vo
     */
    ret = sample_common_svp_start_vi_vpss_venc_vo(&g_vi_config, &g_yolo_switch, &pic_type);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, end_yolo, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),sample_common_svp_start_vi_vpss_venc_vo failed!\n", ret);

    pic_type = PIC_640P;
    ret = sample_comm_sys_get_pic_size(pic_type, &pic_size);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, end_yolo, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),sample_comm_sys_get_pic_size failed!\n", ret);

    /*
     * step 2: Create work thread
     */
    ret = prctl(PR_SET_NAME, "yolo_proc", 0, 0, 0);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, end_yolo, SAMPLE_SVP_ERR_LEVEL_ERROR, "set thread name failed!\n");
    ret = pthread_create(&g_yolo_thread, 0, sample_nnn_proc, NULL);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, end_yolo, SAMPLE_SVP_ERR_LEVEL_ERROR, "pthread_create failed!\n");

    ret = sample_yolo_pause();
    sample_svp_check_exps_return_void(ret == TD_TRUE, SAMPLE_SVP_ERR_LEVEL_ERROR, "yolo exist!\n");
    g_stop_signal = TD_TRUE;
    pthread_join(g_yolo_thread, TD_NULL);
    g_yolo_thread = 0;

end_yolo:
    g_yolo_thread = 0;
    g_stop_signal = TD_TRUE;
    sample_common_svp_stop_vi_vpss_venc_vo(&g_vi_config, &g_yolo_switch);
    return;
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_yolo_handle_sig;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sample_yolo();

    return 0;
}
