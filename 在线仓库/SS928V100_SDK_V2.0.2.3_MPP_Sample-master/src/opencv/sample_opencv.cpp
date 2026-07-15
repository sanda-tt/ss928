/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "sample_common_ive.h"

#include <opencv2/opencv.hpp>

using namespace cv;

#define PIC_SIZE   PIC_3840X2160
#define SAMPLE_STREAM_PATH "./source_file"
#define UHD_STREAN_WIDTH 3840
#define UHD_STREAM_HEIGHT 2160
#define FHD_STREAN_WIDTH 1920
#define FHD_STREAN_HEIGHT 1080
#define REF_NUM 2
#define DISPLAY_NUM 2
#define SAMPLE_VDEC_COMM_VB_CNT 4
#define SAMPLE_VDEC_VPSS_LOW_DELAY_LINE_CNT 16

static ot_payload_type g_cur_type = OT_PT_H264;

static vdec_display_cfg g_vdec_display_cfg = {
    .pic_size = PIC_3840X2160,
    .intf_sync = OT_VO_OUT_1080P60,  //OT_VO_OUT_3840x2160_30
    .intf_type = OT_VO_INTF_HDMI,
};

#define USLEEP_120000   120000
#define USLEEP_60000    60000
#define USLEEP_50       50

#define WIDTH_1080P 1920
#define HEIGHT_1080P 1080

static ot_size g_disp_size;
static td_s32 g_sample_exit = 0;

static pthread_t g_opencv_threadid;
static int g_opencv_start = 0;

#ifndef __LITEOS__
static td_void sample_vdec_handle_sig(td_s32 signo)
{
    if ((signo == SIGINT) || (signo == SIGTERM)) {
        g_sample_exit = 1;
    }
}
#endif

static td_s32 sample_opencv_getchar()
{
    int c;
    if (g_sample_exit == 1) {
        return 'e';
    }

    c = getchar();

    if (g_sample_exit == 1) {
        return 'e';
    }
    return c;
}

static td_u32 sample_vdec_get_chn_width()
{
    switch (g_cur_type) {
        case OT_PT_H264:
        case OT_PT_H265:
            return UHD_STREAN_WIDTH;
        case OT_PT_JPEG:
        case OT_PT_MJPEG:
            return UHD_STREAN_WIDTH;
        default:
            sample_print("invalid type %d!\n", g_cur_type);
            return UHD_STREAN_WIDTH;
    }
}

static td_u32 sample_vdec_get_chn_height()
{
    switch (g_cur_type) {
        case OT_PT_H264:
        case OT_PT_H265:
            return UHD_STREAM_HEIGHT;
        case OT_PT_JPEG:
        case OT_PT_MJPEG:
            return UHD_STREAM_HEIGHT;
        default:
            sample_print("invalid type %d!\n", g_cur_type);
            return UHD_STREAM_HEIGHT;
    }
}

static td_s32 sample_opencv_init_module_vb(sample_vdec_attr *sample_vdec, td_u32 vdec_chn_num, ot_payload_type type,
    td_u32 len)
{
    td_u32 i;
    td_s32 ret;
    for (i = 0; (i < vdec_chn_num) && (i < len); i++) {
        sample_vdec[i].type                           = type;
        sample_vdec[i].width                         = sample_vdec_get_chn_width();
        sample_vdec[i].height                        = sample_vdec_get_chn_height();
        sample_vdec[i].mode                           = sample_comm_vdec_get_lowdelay_en() ? OT_VDEC_SEND_MODE_COMPAT :
                                                        OT_VDEC_SEND_MODE_FRAME;
        sample_vdec[i].sample_vdec_video.dec_mode      = OT_VIDEO_DEC_MODE_IP;
        sample_vdec[i].sample_vdec_video.bit_width     = OT_DATA_BIT_WIDTH_8;
        if (type == OT_PT_JPEG) {
            sample_vdec[i].sample_vdec_video.ref_frame_num = 0;
        } else {
            sample_vdec[i].sample_vdec_video.ref_frame_num = REF_NUM;
        }
        sample_vdec[i].display_frame_num               = DISPLAY_NUM;
        sample_vdec[i].frame_buf_cnt = (type == OT_PT_JPEG) ? (sample_vdec[i].display_frame_num + 1) :
            (sample_vdec[i].sample_vdec_video.ref_frame_num + sample_vdec[i].display_frame_num + 1);
        if (type == OT_PT_JPEG) {
            sample_vdec[i].sample_vdec_picture.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            sample_vdec[i].sample_vdec_picture.alpha      = 255; /* 255:pic alpha value */
        }
    }
    ret = sample_comm_vdec_init_vb_pool(vdec_chn_num, &sample_vdec[0], len);
    if (ret != TD_SUCCESS) {
        sample_print("init mod common vb fail for %#x!\n", ret);
        return ret;
    }
    return ret;
}

static td_s32 sample_opencv_init_sys_and_vb(sample_vdec_attr *sample_vdec, td_u32 vdec_chn_num, ot_payload_type type,
    td_u32 len)
{
    ot_vb_cfg vb_cfg;
    ot_pic_buf_attr buf_attr = {0};
    td_s32 ret;

    ret = sample_comm_sys_get_pic_size(g_vdec_display_cfg.pic_size, &g_disp_size);
    if (ret != TD_SUCCESS) {
        sample_print("sys get pic size fail for %#x!\n", ret);
        return ret;
    }
    buf_attr.align = OT_DEFAULT_ALIGN;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.height = g_disp_size.height;
    buf_attr.width = g_disp_size.width;
    buf_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    (td_void)memset_s(&vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg.max_pool_cnt             = 2;
    vb_cfg.common_pool[0].blk_cnt  = SAMPLE_VDEC_COMM_VB_CNT * vdec_chn_num *2;
    vb_cfg.common_pool[0].blk_size = ot_common_get_pic_buf_size(&buf_attr);

    buf_attr.pixel_format = OT_PIXEL_FORMAT_BGR_888_PLANAR;
    vb_cfg.common_pool[1].blk_cnt  = SAMPLE_VDEC_COMM_VB_CNT * vdec_chn_num;
    vb_cfg.common_pool[1].blk_size = ot_common_get_pic_buf_size(&buf_attr);
    
    ret = sample_comm_sys_init(&vb_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("init sys fail for %#x!\n", ret);
        sample_comm_sys_exit();
        return ret;
    }
    ret = sample_opencv_init_module_vb(&sample_vdec[0], vdec_chn_num, type, len);
    if (ret != TD_SUCCESS) {
        sample_print("init mod vb fail for %#x!\n", ret);
        sample_comm_vdec_exit_vb_pool();
        sample_comm_sys_exit();
        return ret;
    }
    return ret;
}

static td_void sample_opencv_stop_vpss(ot_vpss_grp vpss_grp, td_bool *vpss_chn_enable, td_u32 chn_array_size)
{
    td_s32 i;
    for (i = vpss_grp; i >= 0; i--) {
        vpss_grp = i;
        sample_common_vpss_stop(vpss_grp, &vpss_chn_enable[0], chn_array_size);
    }
}

static td_void sample_opencv_config_vpss_grp_attr(ot_vpss_grp_attr *vpss_grp_attr)
{
    vpss_grp_attr->max_width = sample_vdec_get_chn_width();
    vpss_grp_attr->max_height = sample_vdec_get_chn_height();
    vpss_grp_attr->frame_rate.src_frame_rate = -1;
    vpss_grp_attr->frame_rate.dst_frame_rate = -1;
    vpss_grp_attr->pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vpss_grp_attr->nr_en   = TD_FALSE;
    vpss_grp_attr->ie_en   = TD_FALSE;
    vpss_grp_attr->dci_en   = TD_FALSE;
    vpss_grp_attr->dei_mode = OT_VPSS_DEI_MODE_OFF;
    vpss_grp_attr->buf_share_en   = TD_FALSE;
}

static td_s32 sample_opencv_start_vpss(ot_vpss_grp *vpss_grp, td_u32 vpss_grp_num, td_bool *vpss_chn_enable, td_u32 arr_len)
{
    td_u32 i;
    td_s32 ret;
    ot_vpss_chn_attr vpss_chn_attr[OT_VPSS_MAX_CHN_NUM];
    ot_vpss_grp_attr vpss_grp_attr;

    memset_sp(&vpss_grp_attr, sizeof(ot_vpss_grp_attr), 0, sizeof(ot_vpss_grp_attr));
    sample_opencv_config_vpss_grp_attr(&vpss_grp_attr);
    (td_void)memset_s(vpss_chn_enable, arr_len * sizeof(td_bool), 0, arr_len * sizeof(td_bool));

    vpss_chn_enable[0] = TD_TRUE;
    vpss_chn_attr[0].width         = 1920;
    vpss_chn_attr[0].height        = 1080;
    vpss_chn_attr[0].compress_mode = OT_COMPRESS_MODE_NONE;
    vpss_chn_attr[0].chn_mode                  = OT_VPSS_CHN_MODE_USER;
    vpss_chn_attr[0].pixel_format              = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vpss_chn_attr[0].frame_rate.src_frame_rate = -1;
    vpss_chn_attr[0].frame_rate.dst_frame_rate = -1;
    vpss_chn_attr[0].depth                     = 1;
    vpss_chn_attr[0].mirror_en                 = TD_FALSE;
    vpss_chn_attr[0].flip_en                   = TD_FALSE;
    vpss_chn_attr[0].border_en                 = TD_FALSE;
    vpss_chn_attr[0].aspect_ratio.mode         = OT_ASPECT_RATIO_NONE;

    for (i = 0; i < vpss_grp_num; i++) {
        *vpss_grp = i;
        ret = sample_common_vpss_start(*vpss_grp, &vpss_chn_enable[0],
            &vpss_grp_attr, vpss_chn_attr, OT_VPSS_MAX_CHN_NUM);
        if (ret != TD_SUCCESS) {
            sample_print("start VPSS fail for %#x!\n", ret);
            sample_opencv_stop_vpss(*vpss_grp, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
            return ret;
        }
    }

    return ret;
}

static td_s32 sample_opencv_vpss_unbind_vo(td_u32 vpss_grp_num, sample_vo_cfg vo_config)
{
    td_u32 i;
    ot_vo_layer vo_layer = vo_config.vo_dev;
    td_s32 ret = TD_SUCCESS;
    for (i = 0; i < vpss_grp_num; i++) {
        ret = sample_comm_vpss_un_bind_vo(i, 0, vo_layer, i);
        if (ret != TD_SUCCESS) {
            sample_print("vpss unbind vo fail for %#x!\n", ret);
        }
    }
    return ret;
}

static td_s32 sample_opencv_vpss_bind_vo(sample_vo_cfg vo_config, td_u32 vpss_grp_num)
{
    td_u32 i;
    ot_vo_layer vo_layer;
    td_s32 ret = TD_SUCCESS;
    vo_layer = vo_config.vo_dev;
    for (i = 0; i < vpss_grp_num; i++) {
        ret = sample_comm_vpss_bind_vo(i, 0, vo_layer, i);
        if (ret != TD_SUCCESS) {
            sample_print("vpss bind vo fail for %#x!\n", ret);
            return ret;
        }
    }
    return ret;
}

static td_s32 sample_opencv_start_vo(sample_vo_cfg *vo_config, td_u32 vpss_grp_num)
{
    td_s32 ret;
    vo_config->vo_dev            = SAMPLE_VO_DEV_UHD;
    vo_config->vo_intf_type      = g_vdec_display_cfg.intf_type;
    vo_config->intf_sync         = g_vdec_display_cfg.intf_sync;
    vo_config->pic_size          = g_vdec_display_cfg.pic_size;
    vo_config->bg_color          = COLOR_RGB_BLUE;
    vo_config->dis_buf_len       = 3; /* 3:buf length */
    vo_config->dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    vo_config->vo_mode           = VO_MODE_4MUX;
    vo_config->pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vo_config->disp_rect.x       = 0;
    vo_config->disp_rect.y       = 0;
    vo_config->disp_rect.width   = 1920; //g_disp_size.width;
    vo_config->disp_rect.height  = 1080; //g_disp_size.height;
    vo_config->image_size.width  = 1920; //g_disp_size.width;
    vo_config->image_size.height = 1080; //g_disp_size.height;
    vo_config->vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE;
    vo_config->compress_mode     = OT_COMPRESS_MODE_NONE;

    ret = sample_comm_vo_start_vo(vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("start VO fail for %#x!\n", ret);
        sample_comm_vo_stop_vo(vo_config);
        return ret;
    }

    ret = sample_opencv_vpss_bind_vo(*vo_config, vpss_grp_num);
    if (ret != TD_SUCCESS) {
        sample_opencv_vpss_unbind_vo(vpss_grp_num, *vo_config);
        sample_comm_vo_stop_vo(vo_config);
    }

    return ret;
}

static td_void sample_vdec_cmd_ctrl(td_u32 chn_num, vdec_thread_param *vdec_send, pthread_t *vdec_thread,
    td_u32 send_arr_len, td_u32 thread_arr_len)
{
    td_u32 i;
    ot_vdec_chn_status status;
    td_s32 c;

    for (i = 0; (i < chn_num) && (i < send_arr_len) && (i < thread_arr_len); i++) {
        if (vdec_send[i].circle_send == TD_TRUE) {
            goto circle_send;
        }
    }

    sample_comm_vdec_cmd_not_circle_send(chn_num, vdec_send, vdec_thread, send_arr_len, thread_arr_len);
    return;

circle_send:
    while (1) {
        printf("\n_sample_opencv_test:press 'e' to exit; 'q' to query!;\n");
        c = sample_opencv_getchar();
        if (c == 'e') {
            break;
        } else if (c == 'q') {
            for (i = 0; (i < chn_num) && (i < send_arr_len) && (i < thread_arr_len); i++) {
                ss_mpi_vdec_query_status(vdec_send[i].chn_id, &status);
                sample_comm_vdec_print_chn_status(vdec_send[i].chn_id, status);
            }
        }
    }
    return;
}

static td_void sample_opencv_send_stream_to_vdec(sample_vdec_attr *sample_vdec, td_u32 arr_len, td_u32 vdec_chn_num,
    const char *stream_name)
{
    td_u32 i;
    vdec_thread_param vdec_send[OT_VDEC_MAX_CHN_NUM];
    pthread_t   vdec_thread[OT_VDEC_MAX_CHN_NUM] = {0}; /* 2:thread */
    if (arr_len > OT_VDEC_MAX_CHN_NUM) {
        sample_print("array size(%u) of vdec_send need < %u!\n", arr_len, OT_VDEC_MAX_CHN_NUM);
        return;
    }
    for (i = 0; (i < vdec_chn_num) && (i < arr_len); i++) {
        if (snprintf_s(vdec_send[i].c_file_name, sizeof(vdec_send[i].c_file_name), sizeof(vdec_send[i].c_file_name) - 1,
            stream_name) < 0) {
            return;
        }
        if (snprintf_s(vdec_send[i].c_file_path, sizeof(vdec_send[i].c_file_path), sizeof(vdec_send[i].c_file_path) - 1,
            "%s", SAMPLE_STREAM_PATH) < 0) {
            return;
        }
        vdec_send[i].type          = sample_vdec[i].type;
        vdec_send[i].stream_mode   = sample_vdec[i].mode;
        vdec_send[i].chn_id        = i;
        vdec_send[i].interval_time = 1000; /* 1000: interval time */
        vdec_send[i].pts_init      = 0;
        vdec_send[i].pts_increase  = 0;
        vdec_send[i].e_thread_ctrl = THREAD_CTRL_START;
        vdec_send[i].circle_send   = TD_TRUE;
        vdec_send[i].milli_sec     = 0;
        vdec_send[i].min_buf_size  = (sample_vdec[i].width * sample_vdec[i].height * 3) >> 1; /* 3:yuv */
        vdec_send[i].fps           = 30; /* 30:frame rate */
    }
    sample_comm_vdec_start_send_stream(vdec_chn_num, &vdec_send[0], &vdec_thread[0],
        OT_VDEC_MAX_CHN_NUM, OT_VDEC_MAX_CHN_NUM);

    sample_vdec_cmd_ctrl(vdec_chn_num, &vdec_send[0], &vdec_thread[0],
        OT_VDEC_MAX_CHN_NUM, OT_VDEC_MAX_CHN_NUM);

    sample_comm_vdec_stop_send_stream(vdec_chn_num, &vdec_send[0], &vdec_thread[0],
        OT_VDEC_MAX_CHN_NUM, OT_VDEC_MAX_CHN_NUM);
}

static td_s32 sample_opencv_start_vdec(sample_vdec_attr *sample_vdec, td_u32 vdec_chn_num, td_u32 len)
{
    td_s32 ret;

    ret = sample_comm_vdec_start(vdec_chn_num, &sample_vdec[0], len);
    if (ret != TD_SUCCESS) {
        sample_print("start VDEC fail for %#x!\n", ret);
        sample_comm_vdec_stop(vdec_chn_num);
    }

    return ret;
}

static td_s32 sample_opencv_prepare_frame_info(ot_vb_blk vb_blk, const ot_pic_buf_attr *buf_attr,
    const ot_vb_calc_cfg *calc_cfg, ot_video_frame_info *video_frame)
{
    video_frame->video_frame.header_phys_addr[0] = ss_mpi_vb_handle_to_phys_addr(vb_blk);
    if (video_frame->video_frame.header_phys_addr[0] == TD_NULL) {
        sample_print("ss_mpi_vb_handle_to_phys_addr fail\n");
        return TD_FAILURE;
    }

    video_frame->video_frame.header_virt_addr[0] =
        (td_u8*)ss_mpi_sys_mmap(video_frame->video_frame.header_phys_addr[0], calc_cfg->vb_size);
    if (video_frame->video_frame.header_virt_addr[0] == TD_NULL) {
        sample_print("ss_mpi_sys_mmap fail\n");
        return TD_FAILURE;
    }

    video_frame->mod_id = OT_ID_VGS;
    video_frame->pool_id = ss_mpi_vb_handle_to_pool_id(vb_blk);

    video_frame->video_frame.header_phys_addr[1] =
        video_frame->video_frame.header_phys_addr[0] + calc_cfg->head_y_size;
    video_frame->video_frame.header_virt_addr[1] =
        video_frame->video_frame.header_virt_addr[0] + calc_cfg->head_y_size;
    video_frame->video_frame.phys_addr[0] =
        video_frame->video_frame.header_phys_addr[0] + calc_cfg->head_size;
    video_frame->video_frame.phys_addr[1] =
        video_frame->video_frame.phys_addr[0] + calc_cfg->main_y_size;
    video_frame->video_frame.virt_addr[0] =
        video_frame->video_frame.header_virt_addr[0] + calc_cfg->head_size;
    video_frame->video_frame.virt_addr[1] =
        video_frame->video_frame.virt_addr[0] + calc_cfg->main_y_size;
    video_frame->video_frame.header_stride[0] = calc_cfg->head_stride;
    video_frame->video_frame.header_stride[1] = calc_cfg->head_stride;
    video_frame->video_frame.stride[0] = calc_cfg->main_stride;
    video_frame->video_frame.stride[1] = calc_cfg->main_stride;

    video_frame->video_frame.width = buf_attr->width;
    video_frame->video_frame.height = buf_attr->height;
    video_frame->video_frame.dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    video_frame->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    video_frame->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    video_frame->video_frame.field = OT_VIDEO_FIELD_FRAME;
    video_frame->video_frame.pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    return TD_SUCCESS;
}

static td_void sample_opencv_buf_attr_init(td_u32 width, td_u32 height, ot_pic_buf_attr *buf_attr)
{
    buf_attr->width = width;
    buf_attr->height = height;
    buf_attr->pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    buf_attr->compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr->align = 0;
    buf_attr->bit_width = OT_DATA_BIT_WIDTH_8;
}

/*
 * @brief 将YUV格式的视频帧转换为RGB格式的OpenCV Mat矩阵
 * @param frame 指向视频帧信息的指针
 * @param image 指向OpenCV Mat矩阵的指针
 * @return 返回成功或错误码
 */
static td_s32 ive_yuv_to_rgb(ot_video_frame_info* frame, Mat& image)
{
    // 初始化输入图像的参数
    ot_svp_src_img srcImage;
    memset(&srcImage, 0, sizeof(ot_svp_src_img));
    srcImage.width = frame->video_frame.width;
    srcImage.height = frame->video_frame.height;
    srcImage.type = OT_SVP_IMG_TYPE_YUV420SP;
    srcImage.phys_addr[0] = frame->video_frame.phys_addr[0];
    srcImage.phys_addr[1] = frame->video_frame.phys_addr[1];
    srcImage.stride[0] = frame->video_frame.stride[0];
    srcImage.stride[1] = frame->video_frame.stride[1];

    // 初始化输出图像的参数
    ot_svp_dst_img dstImage;
    sample_common_ive_create_image_by_cached(&dstImage, OT_SVP_IMG_TYPE_U8C3_PACKAGE, frame->video_frame.width, frame->video_frame.height);

    // 初始化颜色空间转换控制参数
    ot_ive_csc_ctrl cscCtrl = {OT_IVE_CSC_MODE_PIC_BT601_YUV_TO_RGB};

    // 创建 IVE 任务句柄
    ot_ive_handle ive_handle;
    int ret = ss_mpi_ive_csc(&ive_handle, &srcImage, &dstImage, &cscCtrl, TD_TRUE);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,"ss mpi ive csc failed,Error(%#x)\n", ret);

    td_u8* ptr_tmp = sample_svp_convert_addr_to_ptr(td_u8, dstImage.virt_addr[0]);
    image.create(frame->video_frame.height, frame->video_frame.width, CV_8UC3);
    memcpy(image.data, (td_u8*)ptr_tmp, frame->video_frame.width * frame->video_frame.height * 3);
    ss_mpi_sys_mmz_free(dstImage.phys_addr[0], dstImage.virt_addr);

    return TD_SUCCESS;
}

/*
 * @brief 将RGB格式的OpenCV Mat矩阵转换为YUV格式的视频帧
 * @param image 指向OpenCV Mat矩阵的指针
 * @param frame 指向视频帧信息的指针
 * @return 返回成功或错误码
 */
static td_s32 ive_rgb_to_yuv(Mat& image, ot_video_frame_info* frame)
{
    // 初始化输出图像的参数
    ot_svp_src_img srcImage;
    sample_common_ive_create_image_by_cached(&srcImage, OT_SVP_IMG_TYPE_U8C3_PACKAGE, image.cols, image.rows);

    // 初始化输出图像的参数
    ot_svp_src_img dstImage;
    memset(&dstImage, 0, sizeof(ot_svp_src_img));
    dstImage.width = image.cols;
    dstImage.height = image.rows;
    dstImage.type = OT_SVP_IMG_TYPE_YUV420SP;
    dstImage.phys_addr[0] = frame->video_frame.phys_addr[0];
    dstImage.phys_addr[1] = frame->video_frame.phys_addr[1];
    dstImage.stride[0] = frame->video_frame.stride[0];
    dstImage.stride[1] = frame->video_frame.stride[1];

    td_u8* ptr_tmp = sample_svp_convert_addr_to_ptr(td_u8, srcImage.virt_addr[0]);
    memcpy((td_u8*)ptr_tmp, image.data, frame->video_frame.width * frame->video_frame.height * 3);

    // 初始化颜色空间转换控制参数
    ot_ive_csc_ctrl cscCtrl = {OT_IVE_CSC_MODE_PIC_BT601_RGB_TO_YUV};

    // 创建 IVE 任务句柄
    ot_ive_handle ive_handle;
    int ret = ss_mpi_ive_csc(&ive_handle, &srcImage, &dstImage, &cscCtrl, TD_TRUE);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,"ss mpi ive csc failed,Error(%#x)\n", ret);

    ss_mpi_sys_mmz_free(srcImage.phys_addr[0], srcImage.virt_addr);

    return TD_SUCCESS;
}

static td_s32 sample_opencv_frame_process(ot_video_frame_info *src_frame, ot_video_frame_info *dst_frame)
{
    /* 1) YUV420SP -> BGR */
    cv::Mat bgr;
    td_s32 ret = ive_yuv_to_rgb(src_frame, bgr);
    if (ret != TD_SUCCESS) {
        sample_print("yuv->rgb failed\n");
        return ret;
    }

    /* 2) OpenCV 伪彩：COLORMAP_JET
       需要 8bit 单通道，先把 BGR 转灰度 */
    cv::Mat gray, pseudo;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::applyColorMap(gray, pseudo, cv::COLORMAP_JET);

    /* 3) BGR 再压回 YUV420SP，复用原 frame 的 buffer */
    ret = ive_rgb_to_yuv(pseudo, dst_frame);
    if (ret != TD_SUCCESS) {
        sample_print("rgb->yuv failed\n");
        return ret;
    }

    /* 4) 保存到文件（不阻塞主线程，可放子线程/定时器） */
#if 0
    static int idx = 0;                     // 简单自增文件名
    cv::imwrite("./opencv_img/bgr_"   + std::to_string(idx) + ".jpg", bgr);
    cv::imwrite("./opencv_img/gray_"  + std::to_string(idx) + ".jpg", gray);
    cv::imwrite("./opencv_img/pseudo_"+ std::to_string(idx) + ".jpg", pseudo);
    ++idx;
#endif
    return TD_SUCCESS;
}

void sample_opencv_set_thread_exit(int exit)
{
    g_opencv_start = exit;
}

int sample_opencv_get_thread_exit(void)
{
    return g_opencv_start;
}

static cv::Mat create_color_checker_fhd()
{
    const int square = 120;               // 每格 120×120
    const int nx = WIDTH_1080P / square;        // 16 格
    const int ny = HEIGHT_1080P / square;        // 9  格

    cv::Mat img(HEIGHT_1080P, WIDTH_1080P, CV_8UC3, cv::Scalar(0,0,0));

    /* 彩虹色循环 */
    const cv::Scalar colors[6] = {
        cv::Scalar(255,   0,   0),   // B
        cv::Scalar(255, 128,   0),   // Cyan
        cv::Scalar(  0, 255,   0),   // G
        cv::Scalar(  0, 255, 255),   // Yellow
        cv::Scalar(  0,   0, 255),   // R
        cv::Scalar(255,   0, 255)    // Magenta
    };

    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            cv::Rect roi(x * square, y * square, square, square);
            img(roi) = colors[(x + y) % 6];
        }
    }
    return img;
}

static void *sample_opencv_thread(void *arg)
{
    int ret;
    ot_vpss_grp src_grp = 0;
    ot_vpss_grp dst_grp = 1;
    ot_video_frame_info src_frame;
    ot_video_frame_info dst_frame;
    ot_vpss_chn_attr chn_attr;
    ot_vb_blk vb_blk;
    ot_pic_buf_attr buf_attr;
    ot_vb_calc_cfg calc_cfg;
    cv::Mat bgr_img;
    cv::Mat g_base = create_color_checker_fhd();
    static int col = 0;                // 当前白色列
    const int sq   = 120;              // 每个方格大小
    const int nx   = WIDTH_1080P / sq;        // 16 格

    memset_sp(&dst_frame, sizeof(ot_video_frame_info), 0, sizeof(ot_video_frame_info));

    ret = ss_mpi_vpss_get_chn_attr(src_grp, 0, &chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vpss_get_chn_attr fail for %#x!\n", ret);
        return NULL;
    }

    sample_opencv_buf_attr_init(chn_attr.width, chn_attr.height, &buf_attr);
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_blk = ss_mpi_vb_get_blk(OT_VB_INVALID_POOL_ID, calc_cfg.vb_size, TD_NULL);
    if (vb_blk == OT_VB_INVALID_HANDLE) {
        sample_print("get vb blk(size:%d) failed!\n", calc_cfg.vb_size);
        return NULL;
    }

    ret = sample_opencv_prepare_frame_info(vb_blk, &buf_attr, &calc_cfg, &dst_frame);
    if (ret != TD_SUCCESS) {
        sample_print("sample_uvc_prepare_frame_info fail for %#x!\n", ret);
        return NULL;
    }

    while (sample_opencv_get_thread_exit()) {
#if 1
        /* ---------- 动态滚动开始 ---------- */
        cv::Mat dyn = g_base.clone();                   // 浅拷贝
        cv::Rect roi(col * sq, 0, sq, HEIGHT_1080P);            // 整列高
        dyn(roi).setTo(cv::Scalar(255,255,255));        // 刷成白色
        /* ---------- 动态滚动结束 ---------- */

        ive_rgb_to_yuv(dyn, &dst_frame);

        ret = ss_mpi_vpss_send_frame(dst_grp, &dst_frame, 100);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vpss_send_frame fail for %#x!\n", ret);
        }
        col = (col + 1) % nx;
#endif

        ret = ss_mpi_vpss_get_chn_frame(src_grp, 0, &src_frame, 100);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vpss_get_chn_frame fail for %#x!\n", ret);
            continue;
        }

        td_s32 ret = ive_yuv_to_rgb(&src_frame, bgr_img);
        if (ret != TD_SUCCESS) {
            sample_print("yuv->rgb failed\n");
        } else {
            static int idx = 10; 
            if (idx > 0) {
                cv::imwrite("./opencv_img/bgr_"   + std::to_string(10-idx) + ".jpg", bgr_img);
                idx--;
            }
        }

        //sample_opencv_frame_process(&src_frame, &dst_frame);
        ret = ss_mpi_vpss_release_chn_frame(src_grp, 0, &src_frame);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vpss_release_chn_frame fail for %#x!\n", ret);
        }
    }

    ss_mpi_sys_munmap(dst_frame.video_frame.virt_addr[0], calc_cfg.vb_size);
    ret = ss_mpi_vb_release_blk(vb_blk);
    if (ret != TD_SUCCESS) {
        sample_print("release vb failed!\n");
        return NULL;
    }

    return NULL;
}

int sample_opencv_task_init(void)
{
    sample_opencv_set_thread_exit(1);
    pthread_create(&g_opencv_threadid, NULL, sample_opencv_thread, NULL);

    return 0;
}

int sample_opencv_task_exit(void)
{
    sample_opencv_set_thread_exit(0);
    pthread_join(g_opencv_threadid, 0);

    return 0;
}

static td_s32 sample_opencv_h264_vdec_vpss_vo(td_void)
{
    td_s32 ret;
    td_u32 vdec_chn_num, vpss_grp_num, i;
    ot_vpss_grp vpss_grp[2];
    sample_vdec_attr sample_vdec[OT_VDEC_MAX_CHN_NUM];
    sample_vo_cfg vo_config;
    td_bool vpss_chn_enable[OT_VPSS_MAX_CHN_NUM];

    vdec_chn_num = 1;
    vpss_grp_num = 2;
    g_cur_type = OT_PT_H264;
    /************************************************
    step1:  init SYS, init common VB(for VPSS and VO), init module VB(for VDEC)
    *************************************************/
    ret = sample_opencv_init_sys_and_vb(&sample_vdec[0], vdec_chn_num, g_cur_type, OT_VDEC_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    /************************************************
    step2:  init VDEC
    *************************************************/
    ret = sample_opencv_start_vdec(&sample_vdec[0], vdec_chn_num, OT_VDEC_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto stop_sys;
    }
    /************************************************
    step3:  start VPSS
    *************************************************/
    ret = sample_opencv_start_vpss(vpss_grp, vpss_grp_num, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto stop_vdec;
    }

    ret = sample_comm_vdec_bind_vpss(0, 0);
    if (ret != TD_SUCCESS) {
        sample_print("vdec bind vpss fail for %#x!\n", ret);
        goto stop_vdec;
    }

    /************************************************
    step4:  start VO
    *************************************************/
    ret = sample_opencv_start_vo(&vo_config, vpss_grp_num);
    if (ret != TD_SUCCESS) {
        goto stop_vpss;
    }

    sample_opencv_task_init();
    /************************************************
    step5:  send stream to VDEC
    *************************************************/
    sample_opencv_send_stream_to_vdec(&sample_vdec[0], OT_VDEC_MAX_CHN_NUM, vdec_chn_num, "3840x2160_8bit.h264");
    sample_opencv_task_exit();

    ret = sample_opencv_vpss_unbind_vo(vpss_grp_num, vo_config);
    sample_comm_vo_stop_vo(&vo_config);
stop_vpss:
    ret = sample_comm_vdec_un_bind_vpss(0, 0);
    if (ret != TD_SUCCESS) {
        sample_print("vdec unbind vpss fail for %#x!\n", ret);
    }

    for (i = 0; i < vpss_grp_num; i++) {
        sample_common_vpss_stop(i, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
    }
stop_vdec:
    sample_comm_vdec_stop(vdec_chn_num);
    sample_comm_vdec_exit_vb_pool();
stop_sys:
    sample_comm_sys_exit();

    return ret;
}

/******************************************************************************
* function    : main()
* description : video vdec sample
******************************************************************************/
#ifdef __LITEOS__
    int app_main(int argc, char *argv[])
#else
    int main(int argc, char *argv[])
#endif
{
    td_s32 ret;

#ifndef __LITEOS__
    sample_sys_signal(sample_vdec_handle_sig);
#endif

    /******************************************
     choose the case
    ******************************************/
    ret = sample_opencv_h264_vdec_vpss_vo();
    if (ret == TD_SUCCESS && g_sample_exit == 0) {
        sample_print("program exit normally!\n");
    } else {
        sample_print("program exit abnormally!\n");
    }

    return ret;
}
