#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <limits.h>
#include "config.h"
#include "log.h"
#include "ot_object_track_define.h"
#include "ot_type.h"
#include "securec.h"
#include "ss_mpi_sys.h"
#include "ss_mpi_vb.h"
#include "ss_object_track.h"
#include "sot.h"

#define SAMPLE_ALG_ALIGN_BYTES_16 16

#define SAMPLE_OBJECT_TRACK_MODEL_PATH "model/HiAI-SOT-Base.om"
#define SAMPLE_OBJECT_TRACK_TRACK_THRESH 0.5
#define SAMPLE_OBJECT_TRACK_YUV_CHN_NUM 3

static OT_OBJECTTRACK_AlgParam g_objectTrackAlgParam = {
    .onlineUpdateThresh =0.9,
    .templateFactor = 2.0,
    .searchFactor = 4.5,
    .imgHeight = 640,
    .imgWidth = 640,
    .onlineUpdateInter = 30,
    .pixFormat = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .modelType = OT_OBJECT_TRACK_MODEL_BASE,
};

static td_s32 AlgObjectTrackModelMemInit(const td_char *modelPath, OT_IAA_AlgMemInfo *model)
{
    FILE *fp = TD_NULL;
    td_s32 ret;
    td_void *virtAddr = TD_NULL;

    fp = fopen(modelPath, "rb");
    OT_SOT_RETURN_IF_PTR_NULL(fp, TD_FAILURE);

    ret = fseek(fp, 0L, SEEK_END);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret != -1, FAIL_0, "fseek failed");

    model->size = ftell(fp);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(model->size > 0, FAIL_0, "ftell failed");

    ret = fseek(fp, 0L, SEEK_SET);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret != -1, FAIL_0, "fseek failed");

    ret = ss_mpi_sys_mmz_alloc(&model->physAddr, &virtAddr, "model", TD_NULL, model->size);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL_0, "ss_mpi_sys_mmz_alloc failed");
    model->virtAddr = virtAddr;

    ret = fread(virtAddr, model->size, 1, fp);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == 1, FAIL_1, "fread failed");

    (td_void)fclose(fp);

    return TD_SUCCESS;
FAIL_1:
    (td_void)ss_mpi_sys_mmz_free(model->physAddr, virtAddr);
FAIL_0:
    (td_void)fclose(fp);
    return TD_FAILURE;
}

static inline td_u32 Alg16BytesAlign(td_u32 num)
{
    return ((num + SAMPLE_ALG_ALIGN_BYTES_16 - 1) / SAMPLE_ALG_ALIGN_BYTES_16 * SAMPLE_ALG_ALIGN_BYTES_16);
}

td_s32 AlgInitTrack(OT_IAA_AlgMemInfo *model, td_u32 *trackHandle)
{
    td_s32 ret;
    const td_char *filePath = SAMPLE_OBJECT_TRACK_MODEL_PATH;
    ret = AlgObjectTrackModelMemInit(filePath, model);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "AlgObjectTrackModelMemInit");

    ret = SS_OBJECTTRACK_Init(model);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SS_OBJECTTRACK_Init");

    ret = SS_OBJECTTRACK_CreateTrackHdl(&g_objectTrackAlgParam, trackHandle);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL_0, "SS_OBJECTTRACK_CreateTrackHdl");

    return TD_SUCCESS;
FAIL_0:
    (td_void)SS_OBJECTTRACK_Deinit();
FAIL:
    (td_void)ss_mpi_sys_mmz_free(model->physAddr, model->virtAddr);

    return ret;
}

td_s32 AlgObjectTrack_ReadPreProcessInputData(OT_IAA_AlgMemInfo *mem, ot_video_frame_info *frame_info, td_void *virt_addr)
{
    mem->physAddr = (td_phys_addr_t)frame_info->video_frame.phys_addr[0];
    mem->virtAddr = virt_addr;
    mem->size = (Alg16BytesAlign(frame_info->video_frame.width * sizeof(td_u8)) * frame_info->video_frame.height *
        SAMPLE_OBJECT_TRACK_YUV_CHN_NUM);

    return TD_SUCCESS;
}

td_s32 AlgObjectTrackSetDebugInfo(td_u32 trackHandle)
{
    td_s32 ret;
    OT_OBJECTTRACK_AlgParam algParam = {0};

    ret = SS_OBJECTTRACK_QueryHdlAlgParam(trackHandle, &algParam);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SS_OBJECTTRACK_QueryHdlAlgParam");
    printf("trackHandle[%u] algParam : onlineUpdateThresh[%f] templateFactor[%f] searchFactor[%f] imgHeight[%u] ",
        trackHandle, algParam.onlineUpdateThresh, algParam.templateFactor, algParam.searchFactor, algParam.imgHeight);
    printf("imgWidth[%u] onlineUpdateInter[%u] pixFormat[%d]\n", algParam.imgWidth, algParam.onlineUpdateInter,
        algParam.pixFormat);

    return TD_SUCCESS;
}

td_s32 AlgObjectTrackGetTimeCost(td_u32 trackHandle)
{
    td_s32 ret;
    OT_OBJECTTRACK_DfxTimeCost timeCost = {0};
    ret = SS_OBJECTTRACK_QueryTimeCost(trackHandle, &timeCost);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SS_OBJECTTRACK_QueryTimeCost");
    printf("TrackHandle(%u) yuv2Rgb TimeCost : ave[%llu us] max[%llu us] min[%llu us]\n", trackHandle,
        timeCost.yuv2Rgb.aveTimeCost, timeCost.yuv2Rgb.maxTimeCost, timeCost.yuv2Rgb.minTimeCost);
    printf("TrackHandle(%u) createSea TimeCost : ave[%llu us] max[%llu us] min[%llu us]\n", trackHandle,
        timeCost.createSea.aveTimeCost, timeCost.createSea.maxTimeCost, timeCost.createSea.minTimeCost);
    printf("TrackHandle(%u) npuProc TimeCost : ave[%llu us] max[%llu us] min[%llu us]\n", trackHandle,
        timeCost.npuProc.aveTimeCost, timeCost.npuProc.maxTimeCost, timeCost.npuProc.minTimeCost);
    printf("TrackHandle(%u) postProc TimeCost : ave[%llu us] max[%llu us] min[%llu us]\n", trackHandle,
        timeCost.postProc.aveTimeCost, timeCost.postProc.maxTimeCost, timeCost.postProc.minTimeCost);
    printf("TrackHandle(%u) totalProc TimeCost : ave[%llu us] max[%llu us] min[%llu us]\n", trackHandle,
        timeCost.totalProc.aveTimeCost, timeCost.totalProc.maxTimeCost, timeCost.totalProc.minTimeCost);

    return TD_SUCCESS;
}