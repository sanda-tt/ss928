#include "python_vio.h"

#define VB_BLK_CNT                  15
#define OT_VPSS_CHN_NUM              2

static sample_vo_cfg g_vo_cfg = {
    .vo_dev            = SAMPLE_VO_DEV_UHD,
    .vo_intf_type      = OT_VO_INTF_HDMI,
    .intf_sync         = OT_VO_OUT_1080P60,
    .bg_color          = COLOR_RGB_BLACK,
    .pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect         = {0, 0, 1920, 1080},
    .image_size        = {1920, 1080},
    .vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len       = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode           = VO_MODE_4MUX,
    .compress_mode     = OT_COMPRESS_MODE_NONE,
};

static sample_comm_venc_chn_param g_venc_chn_param = {
    .frame_rate           = 30, /* 30 is a number */
    .stats_time           = 1,  /* 1 is a number */
    .gop                  = 30, /* 30 is a number */
    .venc_size            = {1920, 1080},
    .size                 = PIC_1080P,
    .profile              = 0,
    .is_rcn_ref_share_buf = TD_FALSE,
    .gop_attr             = {
        .gop_mode = OT_VENC_GOP_MODE_NORMAL_P,
        .normal_p = {2},
    },
    .type                 = OT_PT_H265,
    .rc_mode              = SAMPLE_RC_VBR,
};

/*
 * function : Init Vb
 */
static td_s32 sample_vio_sys_init(ot_size *in_size, td_u32 vpss_chn_num)
{
    td_s32 ret;
    td_u32 i;
    ot_vb_cfg vb_cfg = {0};
    ot_pic_buf_attr buf_attr;
    ot_vb_calc_cfg calc_cfg;
    ot_vi_vpss_mode_type mode_type = OT_VI_ONLINE_VPSS_OFFLINE;
    ot_vi_video_mode video_mode = OT_VI_VIDEO_MODE_NORM;

    vb_cfg.max_pool_cnt = 128; /* 128 blks */

    buf_attr.width         = in_size[0].width;
    buf_attr.height        = in_size[0].height;
    buf_attr.align         = OT_DEFAULT_ALIGN;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_cfg.common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg.common_pool[0].blk_cnt  = VB_BLK_CNT;

    for (i = 1; (i < vpss_chn_num) && (i < OT_VB_MAX_COMMON_POOLS); i++) {
        buf_attr.width         = in_size[i].width;
        buf_attr.height        = in_size[i].height;
        buf_attr.align         = OT_DEFAULT_ALIGN;
        buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;

        ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

        vb_cfg.common_pool[i].blk_size = calc_cfg.vb_size;
        vb_cfg.common_pool[i].blk_cnt  = VB_BLK_CNT;
    }

    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, OT_VB_SUPPLEMENT_BNR_MOT_MASK);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_comm_vi_set_vi_vpss_mode(mode_type, video_mode);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

/*
 * function : vi cfg
 */
static td_void sample_vi_get_one_sensor_vi_cfg(sample_sns_type sns_type, sample_vi_cfg *vi_cfg, int sensor_num)
{
    if (sensor_num == 0) {
        const ot_vi_dev vi_dev = 0; /* dev0 for sensor0 */
        const ot_vi_pipe vi_pipe = 0; /* dev0 bind pipe0 */

        sample_comm_vi_get_default_vi_cfg(sns_type, vi_cfg);

        vi_cfg->mipi_info.divide_mode = LANE_DIVIDE_MODE_1;  //4 lane + 4lane

        vi_cfg->sns_info.bus_id = 5; /* i2c5 */
        vi_cfg->sns_info.sns_clk_src = 0;
        vi_cfg->sns_info.sns_rst_src = 0;

        sample_comm_vi_get_mipi_info_by_dev_id(sns_type, vi_dev, &vi_cfg->mipi_info);
        vi_cfg->dev_info.vi_dev = vi_dev;
        vi_cfg->bind_pipe.pipe_id[0] = vi_pipe;
        vi_cfg->grp_info.grp_num = 1;
        vi_cfg->grp_info.fusion_grp[0] = 0;
        vi_cfg->grp_info.fusion_grp_attr[0].pipe_id[0] = vi_pipe;

    } else if (sensor_num == 1) {
        const ot_vi_dev vi_dev = 2; /* dev2 for sensor1 */
        const ot_vi_pipe vi_pipe = 0; /* dev2 bind pipe0 */

        sample_comm_vi_get_default_vi_cfg(sns_type, vi_cfg);

        vi_cfg->mipi_info.divide_mode = LANE_DIVIDE_MODE_1;  //4 lane + 4lane

        vi_cfg->sns_info.bus_id = 7; /* i2c7 */
        vi_cfg->sns_info.sns_clk_src = 1;
        vi_cfg->sns_info.sns_rst_src = 1;

        sample_comm_vi_get_mipi_info_by_dev_id(sns_type, vi_dev, &vi_cfg->mipi_info);
        vi_cfg->dev_info.vi_dev = vi_dev;
        vi_cfg->bind_pipe.pipe_id[0] = vi_pipe;
        vi_cfg->grp_info.grp_num = 1;
        vi_cfg->grp_info.fusion_grp[0] = 0;
        vi_cfg->grp_info.fusion_grp_attr[0].pipe_id[0] = vi_pipe;
    }
}

static td_s32 sample_vio_start_vi_bind_multi_vpss(td_s32 vpss_grp_cnt, td_s32 vi_chn_cnt, td_s32 vi_chn_interval)
{
    td_s32 ret;
    td_s32 loop;
    ot_vi_chn vi_chn;
    ot_vpss_grp vpss_grp = 0;

    for (loop = 0; loop < vi_chn_cnt  && vpss_grp < vpss_grp_cnt; loop++) {
        vi_chn = loop * vi_chn_interval;
        ret = sample_comm_vi_bind_vpss(0, vi_chn, vpss_grp, 0);
        if (ret != TD_SUCCESS) {
            sample_print("vi bind vpss failed!\n");
            return ret;
        }
        vpss_grp++;
    }

    return TD_SUCCESS;
}

static td_s32 sample_common_svp_vi_unbind_multi_vpss(td_s32 vpss_grp_cnt, td_s32 vi_chn_cnt,
    td_s32 vi_chn_interval)
{
    td_s32 ret;
    td_s32 loop;
    ot_vi_chn vi_chn;
    ot_vpss_grp vpss_grp = 0;

    for (loop = 0; loop < vi_chn_cnt && vpss_grp < vpss_grp_cnt; loop++) {
        vi_chn = loop * vi_chn_interval;
        ret = sample_comm_vi_un_bind_vpss(0, vi_chn, vpss_grp, 0);
        if (ret != TD_SUCCESS) {
            sample_print("vi bind vpss failed!\n");
            return ret;
        }

        vpss_grp++;
    }

    return TD_SUCCESS;
}

static td_s32 sample_vio_start_vpss(td_s32 vpss_grp_cnt, ot_size *in_size, td_u32 vpss_chn_num)
{
    td_u32 i;
    ot_vpss_chn_attr vpss_chn_attr[OT_VPSS_MAX_CHN_NUM];
    ot_vpss_grp_attr vpss_grp_attr;
    td_bool chn_enable[OT_VPSS_MAX_CHN_NUM] = { TD_TRUE, TD_TRUE, TD_FALSE, TD_FALSE };
    ot_vpss_grp vpss_grp;
    td_s32 ret;

    (td_void)memset_s(&vpss_grp_attr, sizeof(ot_vpss_grp_attr), 0, sizeof(ot_vpss_grp_attr));
    sample_comm_vpss_get_default_grp_attr(&vpss_grp_attr);
    vpss_grp_attr.max_width = in_size[0].width;
    vpss_grp_attr.max_height = in_size[0].height;
    vpss_grp_attr.nr_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    /* VPSS only onle channel0 support compress seg mode */
    sample_comm_vpss_get_default_chn_attr(&vpss_chn_attr[0]);
    vpss_chn_attr[0].width = in_size[0].width;
    vpss_chn_attr[0].height = in_size[0].height;
    vpss_chn_attr[0].compress_mode = OT_COMPRESS_MODE_NONE;
    vpss_chn_attr[0].depth = 1;

    for (i = 1; i < vpss_chn_num; i++) {
        (td_void)memset_s(&vpss_chn_attr[i], sizeof(ot_vpss_chn_attr), 0, sizeof(ot_vpss_chn_attr));
        sample_comm_vpss_get_default_chn_attr(&vpss_chn_attr[i]);
        vpss_chn_attr[i].width = in_size[i].width;
        vpss_chn_attr[i].height = in_size[i].height;
        vpss_chn_attr[i].compress_mode = OT_COMPRESS_MODE_NONE;
        vpss_chn_attr[i].depth = 1;
    }

    for (vpss_grp = 0; vpss_grp < vpss_grp_cnt; vpss_grp++) {
        ret = sample_common_vpss_start(vpss_grp, chn_enable, &vpss_grp_attr, vpss_chn_attr, OT_VPSS_MAX_CHN_NUM);
        if (ret != TD_SUCCESS) {
            sample_print("failed with %#x!\n", ret);
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_void sample_vio_stop_vpss(td_s32 vpss_grp_cnt, td_u32 vpss_chn_num)
{
    ot_vpss_grp vpss_grp = 0;
    td_bool chn_enable[OT_VPSS_MAX_CHN_NUM] = { TD_FALSE, TD_FALSE, TD_FALSE, TD_FALSE };
    td_s32 i;
    for (i = 0; (i < vpss_chn_num) && (i < OT_VPSS_MAX_CHN_NUM); i++) {
        chn_enable[i] = TD_TRUE;
    }
    for (i = 0; (i < vpss_grp_cnt) && (i < OT_VPSS_MAX_CHN_NUM); i++) {
        sample_common_vpss_stop(vpss_grp, chn_enable, OT_VPSS_MAX_CHN_NUM);
        vpss_grp++;
    }
}

/*
 * function : Start Vo
 */
static td_s32 sample_vio_start_vo(const sample_qt_switch_t *vo_venc_switch, td_u32 chn_num)
{
    td_s32 ret;
    td_u32 i;
    ot_vpss_grp vpss_grp = 0;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */

    if (vo_venc_switch->is_vo_open == TD_FALSE) {
        return TD_SUCCESS;
    }

    ret = sample_comm_vo_start_vo(&g_vo_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("failed with %#x!\n", ret);
        return TD_FAILURE;
    }

    for (i = 0; i < 1; i++) {  //chn_num
        sample_comm_vpss_bind_vo(vpss_grp, i, vo_layer, vo_chn[i]);
    }

    return ret;
}

/*
 * function : Stop Vo
 */
static td_void sample_vio_stop_vo(const sample_qt_switch_t *vo_venc_switch, td_u32 chn_num)
{
    td_u32 i;
    ot_vpss_grp vpss_grp = 0;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */

    if (vo_venc_switch->is_vo_open == TD_FALSE) {
        return;
    }

    for (i = 0; i < chn_num; i++) {
        sample_comm_vpss_un_bind_vo(vpss_grp, i, vo_layer, vo_chn[i]);
    }

    sample_comm_vo_stop_vo(&g_vo_cfg);
}

/*
 * function : Start Venc
 */
static td_s32 sample_vio_start_venc(const sample_qt_switch_t *vo_venc_switch, const ot_size *in_size, td_u32 chn_num)
{
    td_s32 i, ret;
    ot_vpss_grp vpss_grp = 0;
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    if (vo_venc_switch->is_venc_open == TD_FALSE) {
        return TD_SUCCESS;
    }

    g_venc_chn_param.venc_size.width  = in_size[0].width;
    g_venc_chn_param.venc_size.height = in_size[0].height;
    g_venc_chn_param.size = sample_comm_sys_get_pic_enum(in_size);

    for (i = 0; i < (td_s32)chn_num; i++) {
        ret = sample_comm_venc_start(venc_chn[i], &g_venc_chn_param);
        if (ret != TD_SUCCESS) {
            goto exit;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, chn_num);
    if (ret != TD_SUCCESS) {
        goto exit;
    }

    for (i = 0; i < chn_num; i++) {
        sample_comm_vpss_bind_venc(vpss_grp, i, venc_chn[i]);
    }

    return TD_SUCCESS;

exit:
    for (i = i - 1; i >= 0; i--) {
        sample_comm_venc_stop(venc_chn[i]);
    }
    return TD_FAILURE;
}

/*
 * function : Stop Venc
 */
static td_void sample_vio_stop_venc(const sample_qt_switch_t *vo_venc_switch, td_u32 chn_num)
{
    td_u32 i;
    ot_vpss_grp vpss_grp = 0;
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    if (vo_venc_switch->is_venc_open == TD_FALSE) {
        return ;
    }

    for (i = 0; i < chn_num; i++) {
        sample_comm_vpss_un_bind_venc(vpss_grp, i, venc_chn[i]);
    }

    sample_comm_venc_stop_get_stream(chn_num);

    for (i = 0; i < chn_num; i++) {
        sample_comm_venc_stop(venc_chn[i]);
    }
}

/*
    1路 4lane sensor

    已适配 imx347   4lane  sensor0/1
    已适配 os08a20  4lane  sensor0/1
    已适配 os04a10  4lane  sensor0/1
    已适配 sc450ai  4lane  sensor0/1
*/
td_s32 sample_vio_start_one_sensor(int sensor_num, sample_vi_cfg *vi_cfg, sample_qt_switch_t *switch_ptr)
{
    td_s32 ret;
    const td_u32 vpss_grp_cn = 1;
    sample_sns_type sns_type = SENSOR0_TYPE;
    ot_size in_size[OT_VPSS_CHN_NUM];

    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size[0]);

    /* set vpss chn1 size */
    ot_pic_size ext_pic_size_type = PIC_640P;
    sample_comm_sys_get_pic_size(ext_pic_size_type, &in_size[1]);

    /* step  1: Init vb */
    ret = sample_vio_sys_init(in_size, OT_VPSS_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto sys_init_failed;
    }

    /* step 2: Start vi */
    sample_vi_get_one_sensor_vi_cfg(sns_type, vi_cfg, sensor_num);
    ret = sample_comm_vi_start_vi(vi_cfg);
        if (ret != TD_SUCCESS) {
        goto start_vi_failed;
    }

    /* step 3: Bind vpss to vi */
    ret = sample_vio_start_vi_bind_multi_vpss(vpss_grp_cn, 1, 1);
    if (ret != TD_SUCCESS) {
        goto start_vi_bind_vpss_failed;
    }

    /* step 4: Start vpss */
    ret = sample_vio_start_vpss(vpss_grp_cn, in_size, OT_VPSS_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto start_vpss_failed;
    }

    /* step 5: Start Vo*/
    ret = sample_vio_start_vo(switch_ptr, OT_VPSS_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto start_vo_failed;
    }

    /* step 6: Start Venc*/
    ret = sample_vio_start_venc(switch_ptr, in_size, OT_VPSS_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto start_venc_failed;
    }

    return TD_SUCCESS;

start_venc_failed:
    sample_vio_stop_venc(switch_ptr, OT_VPSS_CHN_NUM);
    sample_vio_stop_vo(switch_ptr, OT_VPSS_CHN_NUM);
start_vo_failed:
    sample_vio_stop_vpss(vpss_grp_cn, OT_VPSS_CHN_NUM);
start_vpss_failed:
    sample_common_svp_vi_unbind_multi_vpss(vpss_grp_cn, 1, 1);
start_vi_bind_vpss_failed:
    sample_comm_vi_stop_vi(vi_cfg);
start_vi_failed:
    sample_comm_sys_exit();
sys_init_failed:
    return ret;
}

void sample_vio_stop_one_sensor(sample_vi_cfg *vi_cfg, sample_qt_switch_t *switch_ptr)
{
    const td_u32 vpss_grp_cn = 1;

    sample_vio_stop_venc(switch_ptr, OT_VPSS_CHN_NUM);
    sample_vio_stop_vo(switch_ptr, OT_VPSS_CHN_NUM);

    sample_vio_stop_vpss(vpss_grp_cn, OT_VPSS_CHN_NUM);
    sample_common_svp_vi_unbind_multi_vpss(vpss_grp_cn, 1, 1);

    sample_comm_vi_stop_vi(vi_cfg);
    sample_comm_sys_exit();
}