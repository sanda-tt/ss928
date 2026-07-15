#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "sample_comm.h"
#include "sample_common_ive.h"

#define  MILLI_SEC  20000
#define  OT_SAMPLE_PYMODULE_QUERY_SLEEP         100

namespace py = pybind11;
ot_video_frame_info g_frame_info[OT_VPSS_MAX_GRP_NUM][OT_VPSS_MAX_CHN_NUM] = {0};

static td_s32 sample_pymodule_query_task(ot_ive_handle handle)
{
    td_s32 ret;
    td_bool is_block = TD_TRUE;
    td_bool is_finish = TD_FALSE;

    ret = ss_mpi_ive_query(handle, &is_finish, is_block);
    while (ret == OT_ERR_IVE_QUERY_TIMEOUT) {
        usleep(OT_SAMPLE_PYMODULE_QUERY_SLEEP);
        ret = ss_mpi_ive_query(handle, &is_finish, is_block);
    }
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),ss_mpi_ive_query failed!\n", ret);

    return TD_SUCCESS;
}

/*
 * @brief 将YUV格式的视频帧转换为RGB格式的numpy.ndarray
 */
static td_s32 ive_yuv_to_rgb(ot_video_frame_info* frame, py::array_t<uint8_t>& image_data)
{
    int ret;
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
    ret = ss_mpi_ive_csc(&ive_handle, &srcImage, &dstImage, &cscCtrl, TD_TRUE);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,"ss_mpi_ive_csc failed,Error(%#x)\n", ret);

    ret = sample_pymodule_query_task(ive_handle);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "query_task failed!\n");

    td_u8* ptr_tmp = sample_svp_convert_addr_to_ptr(td_u8, dstImage.virt_addr[0]);

    std::vector<ssize_t> shape   = { (ssize_t)frame->video_frame.height,
                                     (ssize_t)frame->video_frame.width,
                                     3 };
    std::vector<ssize_t> strides = { (ssize_t)(frame->video_frame.width * 3),
                                     (ssize_t)3,
                                     (ssize_t)1 };

    image_data = py::array_t<uint8_t>(shape, strides, ptr_tmp).attr("copy")();

    ss_mpi_sys_mmz_free(dstImage.phys_addr[0], dstImage.virt_addr);

    return TD_SUCCESS;
}

/*
 * @brief 将RGB格式的numpy.ndarray 转换为YUV格式的视频帧
 */
static td_s32 ive_rgb_to_yuv(py::array_t<uint8_t>& image_data, ot_video_frame_info* frame)
{
    int ret;
    py::buffer_info buf = image_data.request();
    if (buf.ndim != 3 || buf.shape[2] != 3) {
        throw std::runtime_error("Expected HxWx3 RGB numpy array");
    }

    // 初始化输出图像的参数
    ot_svp_src_img srcImage;
    sample_common_ive_create_image_by_cached(&srcImage, OT_SVP_IMG_TYPE_U8C3_PACKAGE, buf.shape[1], buf.shape[0]);

    // 初始化输出图像的参数
    ot_svp_src_img dstImage;
    memset(&dstImage, 0, sizeof(ot_svp_src_img));
    dstImage.width = buf.shape[1];
    dstImage.height = buf.shape[0];
    dstImage.type = OT_SVP_IMG_TYPE_YUV420SP;
    dstImage.phys_addr[0] = frame->video_frame.phys_addr[0];
    dstImage.phys_addr[1] = frame->video_frame.phys_addr[1];
    dstImage.stride[0] = frame->video_frame.stride[0];
    dstImage.stride[1] = frame->video_frame.stride[1];

    memcpy_s(sample_svp_convert_addr_to_ptr(td_u8, srcImage.virt_addr[0]), buf.shape[0] * buf.shape[1] * 3, buf.ptr, buf.shape[0] * buf.shape[1] * 3);

    // 初始化颜色空间转换控制参数
    ot_ive_csc_ctrl cscCtrl = {OT_IVE_CSC_MODE_PIC_BT601_RGB_TO_YUV};

    // 创建 IVE 任务句柄
    ot_ive_handle ive_handle;
    ret = ss_mpi_ive_csc(&ive_handle, &srcImage, &dstImage, &cscCtrl, TD_TRUE);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,"ss_mpi_ive_csc failed,Error(%#x)\n", ret);

    ret = sample_pymodule_query_task(ive_handle);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "query_task failed!\n");

    ss_mpi_sys_mmz_free(srcImage.phys_addr[0], srcImage.virt_addr);

    return TD_SUCCESS;
}

/*
 * @brief Python接口：用户从VPSS通道获取一帧处理完成的图像并转成RGB numpy.ndarray
 */

static py::array_t<uint8_t> py_vpss_get_chn_frame(int grp_num, int chn_num)
{
    td_s32 ret;
    ot_vpss_grp vpss_grp = grp_num;
    ot_vpss_chn vpss_chn = chn_num;

    ret = ss_mpi_vpss_get_chn_frame(vpss_grp, vpss_chn, &g_frame_info[vpss_grp][vpss_chn], MILLI_SEC);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vpss_get_chn_frame err:0x%x\n", ret);
        std::ostringstream oss;
        oss << "ss_mpi_vpss_get_chn_frame failed, ret=0x" << std::hex << std::showbase << ret;
        throw std::runtime_error(oss.str());
    }

    py::array_t<uint8_t> image_data;
    ret = ive_yuv_to_rgb(&g_frame_info[vpss_grp][vpss_chn], image_data);
    if (ret != TD_SUCCESS) {
        sample_print("ive_yuv_to_rgb failed, ret:0x%x\n", ret);
        std::ostringstream oss;
        oss << "ive_yuv_to_rgb failed, ret=0x" << std::hex << std::showbase << ret;
        throw std::runtime_error(oss.str());
    }

    return image_data;
}

/*
 * @brief Python接口：将RGB numpy.ndarray转成YUV格式的视频帧并送入指定输出通道显示
 */

static td_s32 py_vo_send_frame(int vo_layer, int vo_chn, py::array_t<uint8_t>& image_data, int grp_num, int chn_num)
{
    td_s32 ret;
    ot_vo_layer layer = vo_layer;
    ot_vo_chn   chn = vo_chn;
    ot_vpss_grp vpss_grp = grp_num;
    ot_vpss_chn vpss_chn = chn_num;

    ret = ive_rgb_to_yuv(image_data, &g_frame_info[vpss_grp][vpss_chn]);
    if (ret != TD_SUCCESS) {
        sample_print("ive_rgb_to_yuv failed, ret:0x%x\n", ret);
        std::ostringstream oss;
        oss << "ive_rgb_to_yuv failed, ret=0x" << std::hex << std::showbase << ret;
        throw std::runtime_error(oss.str());
    }

    ret = ss_mpi_vo_send_frame(layer, chn, &g_frame_info[vpss_grp][vpss_chn], MILLI_SEC);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vo_send_frame failed, ret:0x%x\n", ret);
        std::ostringstream oss;
        oss << "ss_mpi_vo_send_frame failed, ret=0x" << std::hex << std::showbase << ret;
        throw std::runtime_error(oss.str());
    }

    return TD_SUCCESS;
}

/*
 * @brief Python接口：用户释放一帧VPSS通道图像
 */

static td_s32 py_vpss_release_chn_frame(int grp_num, int chn_num)
{
    td_s32 ret;
    ot_vpss_grp vpss_grp = grp_num;
    ot_vpss_chn vpss_chn = chn_num;

    ret = ss_mpi_vpss_release_chn_frame(vpss_grp, vpss_chn, &g_frame_info[vpss_grp][vpss_chn]);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_vpss_release_chn_frame failed, ret:0x%x\n", ret);
        std::ostringstream oss;
        oss << "ss_mpi_vpss_get_chn_frame failed, ret=0x" << std::hex << std::showbase << ret;
        throw std::runtime_error(oss.str());
    }

    return TD_SUCCESS;
}

/*
 * Python 模块绑定
 */
PYBIND11_MODULE(sample_pymodule, m) {
    m.doc() = "HiSilicon IVE YUV<->RGB conversion, using Python cv2 (numpy ndarray)";

    m.def("py_vpss_get_chn_frame",
           &py_vpss_get_chn_frame,
           py::arg("grp_num"),
           py::arg("chn_num"),
           "Get one RGB frame from VPSS channel");

    m.def("py_vpss_release_chn_frame",
           &py_vpss_release_chn_frame,
           py::arg("grp_num"),
           py::arg("chn_num"),
           "Release one frame from VPSS channel");

    m.def("py_vo_send_frame",
           &py_vo_send_frame,
           py::arg("vo_layer"),
           py::arg("vo_chn"),
           py::arg("image_data"),
           py::arg("grp_num"),
           py::arg("chn_num"),
           "Send RGB numpy.ndarray to VO display layer");
}
