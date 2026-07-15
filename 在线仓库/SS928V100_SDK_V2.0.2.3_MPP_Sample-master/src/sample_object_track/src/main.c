 /*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: object track sample.
 * Author: Software Develop Team
 * Create: 2024-05-13
 */

#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <limits.h>
#include "config.h"
#include "log.h"
#include "ot_type.h"
#include "securec.h"
#include "ss_mpi_sys.h"
#include "ss_mpi_vb.h"
#include "ss_object_track.h"


#define SAMPLE_ALG_ARG_SIZE_3 3
#define SAMPLE_ALG_CONFIG_FILE_INDEX 2
#define SAMPLE_ALG_PATH_LEN 512
#define SAMPLE_ALG_ALIGN_BYTES_16 16
#define SAMPLE_ALG_ALIGN_NUM_2 2
#define SAMPLE_ALG_US_PER_S 1000000
#define SAMPLE_OBJECT_TRACK_IMAGE_MAX_CNT 15000
#define SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN 500

#define SAMPLE_OBJECT_TRACK_OUTPUT_PATH "result/"
#define SAMPLE_OBJECT_TRACK_MODEL_PATH "model/HiAI-SOT-Base.om"
#define SAMPLE_OBJECT_TRACK_RESULT_PATH "result/"
#define SAMPLE_OBJECT_TRACK_TRACK_THRESH 0.5
#define SAMPLE_OBJECT_TRACK_YUV_CHN_NUM 3

static td_bool g_terminateSignal = TD_FALSE;

typedef struct {
    SAMPLE_ControlMode mode;
} SAMPLE_MainAttr;

typedef struct {
    td_char seqName[SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN]; /* 500 : seq name */
    td_u32 width;
    td_u32 height;
} OT_IAA_ObjectTrackSeqInfo;

static SAMPLE_ConfigInfo g_configInfo = {
    .initBoxParam = {18, 240, 932, 754},
    .objectTrackAlgParam = {0.9, 2.0, 4.5, 1080, 1920, 30, OT_PIXEL_FORMAT_RGB_888, OT_OBJECT_TRACK_MODEL_BASE},
    .trackFramePath = "data/rgb/",
    .batchInfoFilePath = "",
};

static OT_OBJECTTRACK_DfxInfo g_objectTrackDfxInfo = {
    .enableDump = TD_TRUE,
    .dumpPath = "result/",
};

static inline td_u32 SAMPLE_Alg16BytesAlign(td_u32 num)
{
    return ((num + SAMPLE_ALG_ALIGN_BYTES_16 - 1) / SAMPLE_ALG_ALIGN_BYTES_16 * SAMPLE_ALG_ALIGN_BYTES_16);
}

static td_s32 SAMPLE_AlgObjectTrack_YuvMemInit(const OT_OBJECTTRACK_AlgParam *algParam, OT_IAA_AlgMemInfo *mem)
{
    td_s32 ret;
    mem->size = (SAMPLE_Alg16BytesAlign(algParam->imgWidth * sizeof(td_u8)) * algParam->imgHeight *
        SAMPLE_OBJECT_TRACK_YUV_CHN_NUM);
    if (algParam->pixFormat == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        mem->size >>= 1;  /* yuv420sp */
    }
    ret = ss_mpi_sys_mmz_alloc_cached(&mem->physAddr, &mem->virtAddr, "objecttrack_yuvin", TD_NULL, mem->size);
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "ss_mpi_sys_mmz_alloc_cached failed");

    (td_void)memset_s(mem->virtAddr, mem->size, 0, mem->size);
    (td_void)ss_mpi_sys_flush_cache(mem->physAddr, mem->virtAddr, mem->size);

    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrack_ReadPreProcessInputData(const OT_OBJECTTRACK_AlgParam *algParam,
    OT_IAA_AlgMemInfo *mem, const td_char *imgPath)
{
    FILE *f1 = fopen(imgPath, "r");
    if (f1 == TD_NULL) {
        LOGE("open file path:%s failed\n", imgPath);
        return TD_FAILURE;
    }
    td_u32 lineNum;
    td_s32 ret;

    lineNum = (algParam->imgHeight * SAMPLE_OBJECT_TRACK_YUV_CHN_NUM);
    if (algParam->pixFormat == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        lineNum >>= 1;  /* yvu 420sp */
    }

    td_u8 *addr = (td_u8 *)mem->virtAddr;
    td_u32 stride = SAMPLE_Alg16BytesAlign(algParam->imgWidth * sizeof(td_u8));
    for (td_u32 i = 0; i < lineNum; i++) {
        ret = fread(addr + i * stride, algParam->imgWidth, 1, f1);
        if (ret != 1) {
            LOGE("fread file path:%s failed ret %d\n", imgPath, ret);
            (td_void)fclose(f1);
            return TD_FAILURE;
        }
    }

    (td_void)fclose(f1);
    (td_void)ss_mpi_sys_flush_cache(mem->physAddr, mem->virtAddr, mem->size);

    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrack_WriteProcessOutputData(const OT_OBJECTTRACK_AlgParam *algParam,
    const OT_IAA_AlgMemInfo *mem, td_u32 frmIdx)
{
    td_s32 ret;
    td_char imgPath[SAMPLE_ALG_PATH_LEN] = {0};
    ret = sprintf_s(imgPath, SAMPLE_ALG_PATH_LEN - 1, "%s%04u%s", SAMPLE_OBJECT_TRACK_OUTPUT_PATH, frmIdx, ".yuv");
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret != -1, ret, "sprintf_s");
    FILE *f1 = fopen(imgPath, "w+");
    if (f1 == TD_NULL) {
        LOGE("open file path:%s failed %s\n", imgPath, strerror(errno));
        return TD_FAILURE;
    }
    td_u32 lineNum;

    lineNum = (algParam->imgHeight * SAMPLE_OBJECT_TRACK_YUV_CHN_NUM);
    if (algParam->pixFormat == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        lineNum >>= 1;  /* yvu 420sp */
    }
    (td_void)ss_mpi_sys_flush_cache(mem->physAddr, mem->virtAddr, mem->size);
    td_u8 *addr = (td_u8 *)mem->virtAddr;
    td_u32 stride = SAMPLE_Alg16BytesAlign(algParam->imgWidth * sizeof(td_u8));
    for (td_u32 i = 0; i < lineNum; i++) {
        ret = fwrite(addr + i * stride, algParam->imgWidth, 1, f1);
        if (ret != 1) {
            LOGE("fread file path:%s failed ret %d\n", imgPath, ret);
            (td_void)fclose(f1);
            return TD_FAILURE;
        }
    }

    (td_void)fclose(f1);

    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgSaveObjectTrackResult(const OT_IAA_ObjectTrackSeqInfo *seqInfo,
    const OT_OBJECTTRACK_Result *res, td_u32 frmIdx)
{
    LOGD("*****track result*****\n");
    LOGD("score : %f\n", res->score);
    LOGD("x : %d  y : %d  w : %d  h :%d\n", res->boxOut.x, res->boxOut.y, res->boxOut.width,
        res->boxOut.height);

    td_char resultPath[SAMPLE_ALG_PATH_LEN];
    td_s32 ret = sprintf_s(resultPath, SAMPLE_ALG_PATH_LEN - 1, "%s%s%s",
        SAMPLE_OBJECT_TRACK_RESULT_PATH, strcmp(seqInfo->seqName, "") == 0 ? "result" : seqInfo->seqName, ".txt");
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret != TD_FAILURE, ret, "sprintf_s");
    FILE *fp = fopen(resultPath, "a+");
    if (fp == TD_NULL) {
        LOGE("fopen %s failed\n", SAMPLE_OBJECT_TRACK_RESULT_PATH);
        return TD_FAILURE;
    }
    td_char buff[SAMPLE_ALG_PATH_LEN] = {0};
    ret = sprintf_s(buff, sizeof(buff), "frame_%08d %d %d %d %d %.6f\n",
        frmIdx, res->boxOut.x, res->boxOut.y, res->boxOut.width + res->boxOut.x,
        res->boxOut.height + res->boxOut.y, res->score);
    (td_void)fwrite(buff, strlen(buff), 1, fp);
    (td_void)fclose(fp);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret != TD_FAILURE, ret, "sprintf_s");
    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrackModelMemInit(const td_char *modelPath, OT_IAA_AlgMemInfo *model)
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

static td_s32 SAMPLE_AlgObjectTrackSaveFirstFrameBox(const OT_IAA_ObjectTrackSeqInfo *seqInfo,
    const OT_OBJECTTRACK_BoxParam *seqBox)
{
    td_char resultPath[SAMPLE_ALG_PATH_LEN];
    g_configInfo.objectTrackAlgParam.imgHeight = seqInfo->height;
    g_configInfo.objectTrackAlgParam.imgWidth = seqInfo->width;

    td_s32 ret = sprintf_s(resultPath, SAMPLE_ALG_PATH_LEN - 1, "%s%s%s",
        SAMPLE_OBJECT_TRACK_RESULT_PATH, strcmp(seqInfo->seqName, "") == 0 ? "result" : seqInfo->seqName, ".txt");
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret != TD_FAILURE, ret, "sprintf_s");
    FILE *fp = fopen(resultPath, "a+");
    if (fp == TD_NULL) {
        printf("fopen %s failed\n", resultPath);
        return TD_FAILURE;
    }
    td_char buff[SAMPLE_ALG_PATH_LEN] = {0};
    td_s32 x = (td_s32)seqBox->x;
    td_s32 y = (td_s32)seqBox->y;
    td_s32 w = (td_s32)seqBox->width;
    td_s32 h = (td_s32)seqBox->height;
    ret = sprintf_s(buff, sizeof(buff), "frame_00000000 %d %d %d %d 1.0\n", x, y, w + x, h + y);

    (td_void)fwrite(buff, strlen(buff), 1, fp);
    (td_void)fclose(fp);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret != TD_FAILURE, ret, "sprintf_s");
    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgInitTrack(OT_IAA_AlgMemInfo *model, td_u32 *trackHandle, OT_IAA_AlgMemInfo *memYuvIn)
{
    td_s32 ret;
    const td_char *filePath = SAMPLE_OBJECT_TRACK_MODEL_PATH;
    ret = SAMPLE_AlgObjectTrackModelMemInit(filePath, model);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SAMPLE_AlgObjectTrackModelMemInit");

    ret = SS_OBJECTTRACK_Init(model);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SS_OBJECTTRACK_Init");

    ret = SAMPLE_AlgObjectTrack_YuvMemInit(&g_configInfo.objectTrackAlgParam, memYuvIn);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL_0, "SAMPLE_AlgObjectTrack_YuvMemInit");

    ret = SS_OBJECTTRACK_CreateTrackHdl(&g_configInfo.objectTrackAlgParam, trackHandle);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL_1, "SS_OBJECTTRACK_CreateTrackHdl");

    return TD_SUCCESS;
FAIL_1:
    (td_void)ss_mpi_sys_mmz_free(memYuvIn->physAddr, memYuvIn->virtAddr);
FAIL_0:
    (td_void)SS_OBJECTTRACK_Deinit();
FAIL:
    (td_void)ss_mpi_sys_mmz_free(model->physAddr, model->virtAddr);

    return ret;
}

static td_void SAMPLE_AlgFillVgsParam(const OT_OBJECTTRACK_AlgParam *algParam, const OT_IAA_AlgMemInfo *memYuvIn,
    const OT_OBJECTTRACK_Result *res, ot_vgs_task_attr *taskAttr, ot_corner_rect *cornerRect)
{
    td_u32 stride;
    stride = SAMPLE_Alg16BytesAlign(algParam->imgWidth);
    taskAttr->img_in.pool_id = 0;
    taskAttr->img_in.mod_id = OT_ID_CMPI;
    taskAttr->img_in.video_frame.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    taskAttr->img_in.video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    taskAttr->img_in.video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    taskAttr->img_in.video_frame.width = algParam->imgWidth;
    taskAttr->img_in.video_frame.height = algParam->imgHeight;
    taskAttr->img_in.video_frame.phys_addr[0] = memYuvIn->physAddr;
    taskAttr->img_in.video_frame.phys_addr[1] = memYuvIn->physAddr + algParam->imgHeight * stride;
    taskAttr->img_in.video_frame.virt_addr[0] = memYuvIn->virtAddr;
    taskAttr->img_in.video_frame.virt_addr[1] = (td_u8 *)memYuvIn->virtAddr + algParam->imgHeight * stride;
    taskAttr->img_in.video_frame.stride[0] = stride;
    taskAttr->img_in.video_frame.stride[1] = stride;
    taskAttr->img_in.video_frame.dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    taskAttr->img_in.video_frame.field = OT_VIDEO_FIELD_FRAME;
    taskAttr->img_out = taskAttr->img_in;

    cornerRect->rect.x = res->boxOut.x / SAMPLE_ALG_ALIGN_NUM_2 * SAMPLE_ALG_ALIGN_NUM_2;
    cornerRect->rect.y = res->boxOut.y / SAMPLE_ALG_ALIGN_NUM_2 * SAMPLE_ALG_ALIGN_NUM_2;
    cornerRect->rect.width = res->boxOut.width / SAMPLE_ALG_ALIGN_NUM_2 * SAMPLE_ALG_ALIGN_NUM_2;
    cornerRect->rect.height = res->boxOut.height / SAMPLE_ALG_ALIGN_NUM_2 * SAMPLE_ALG_ALIGN_NUM_2;
    cornerRect->thick = 4; /* 4 : pix */
}

static td_s32 SAMPLE_AlgDrawResBox(const OT_OBJECTTRACK_AlgParam *algParam, const OT_IAA_AlgMemInfo *memYuvIn,
    const OT_OBJECTTRACK_Result *res, td_u32 frmIdx)
{
    if (algParam->pixFormat != OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        LOGD("only yvu420sp can draw res box!\n");
        return TD_SUCCESS;
    }
    td_s32 ret;
    td_u32 boxNum = 1;
    ot_vgs_task_attr taskAttr = {0};
    ot_corner_rect cornerRect = {0};
    ot_corner_rect_attr rectAttr = {0};
    ot_vgs_handle handle = -1;
    ret = ss_mpi_vgs_begin_job(&handle);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "ss_mpi_vgs_begin_job");
    SAMPLE_AlgFillVgsParam(algParam, memYuvIn, res, &taskAttr, &cornerRect);

    rectAttr.corner_rect_type = OT_CORNER_RECT_TYPE_FULL_LINE;
    if (res->score > SAMPLE_OBJECT_TRACK_TRACK_THRESH) {
        rectAttr.color = 0x00FF00; /* green yvu420sp */
    } else {
        rectAttr.color = 0x0000FF; /* red yvu420sp */
    }

    ret = ss_mpi_vgs_add_corner_rect_task(handle, &taskAttr, &cornerRect, &rectAttr, boxNum);
    if (ret != TD_SUCCESS) {
        (td_void)ss_mpi_vgs_cancel_job(handle);
        LOGE("ss_mpi_vgs_add_corner_rect_task failed 0x%x\n", ret);
        return TD_FAILURE;
    }

    ret = ss_mpi_vgs_end_job(handle);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "ss_mpi_vgs_end_job");

    ret = SAMPLE_AlgObjectTrack_WriteProcessOutputData(algParam, memYuvIn, frmIdx);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "write yuv");

    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrackSetDebugInfo(td_u32 trackHandle, const SAMPLE_MainAttr *mainAttr)
{
    if (mainAttr->mode != DEBUG_MODE) {
        return TD_SUCCESS;
    }

    td_s32 ret;
    OT_OBJECTTRACK_DfxInfo dfxInfo = {0};
    OT_OBJECTTRACK_AlgParam algParam = {0};

    ret = SS_OBJECTTRACK_SetHdlDfxInfo(trackHandle, &g_objectTrackDfxInfo);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SS_OBJECTTRACK_SetHdlDfxInfo");

    ret = SS_OBJECTTRACK_GetHdlDfxInfo(trackHandle, &dfxInfo);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SS_OBJECTTRACK_GetHdlDfxInfo");
    LOGI("trackHandle[%u] dfxInfo : enableDump[%d] dumpPath[%s]\n", trackHandle, dfxInfo.enableDump, dfxInfo.dumpPath);

    ret = SS_OBJECTTRACK_QueryHdlAlgParam(trackHandle, &algParam);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SS_OBJECTTRACK_QueryHdlAlgParam");
    printf("trackHandle[%u] algParam : onlineUpdateThresh[%f] templateFactor[%f] searchFactor[%f] imgHeight[%u] ",
        trackHandle, algParam.onlineUpdateThresh, algParam.templateFactor, algParam.searchFactor, algParam.imgHeight);
    printf("imgWidth[%u] onlineUpdateInter[%u] pixFormat[%d]\n", algParam.imgWidth, algParam.onlineUpdateInter,
        algParam.pixFormat);

    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrackPostProc(const OT_IAA_ObjectTrackSeqInfo *seqInfo, td_u32 frmIdx,
    const SAMPLE_MainAttr *mainAttr, const OT_IAA_AlgMemInfo *memYuvIn, const OT_OBJECTTRACK_Result *trackRes)
{
    td_s32 ret;
    ret = SAMPLE_AlgSaveObjectTrackResult(seqInfo, trackRes, frmIdx);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SAMPLE_AlgSaveObjectTrackResult");

    if (mainAttr->mode == DEBUG_MODE) {
        ret = SAMPLE_AlgDrawResBox(&g_configInfo.objectTrackAlgParam, memYuvIn, trackRes, frmIdx);
        OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SAMPLE_AlgDrawResBox");
    }

    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrackGetTimeCost(td_u32 trackHandle)
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

static td_s32 SAMPLE_CheckFile(const char *path)
{
    FILE *f1 = fopen(path, "r");
    if (f1 == TD_NULL) {
        return TD_FAILURE;
    }
    (td_void)fclose(f1);
    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgObjectTrack(const OT_IAA_ObjectTrackSeqInfo *seqInfo, const OT_OBJECTTRACK_BoxParam *seqBox,
    const SAMPLE_MainAttr *mainAttr)
{
    td_s32 ret = 0;
    td_u32 trackHandle;
    OT_IAA_AlgMemInfo memYuvIn, model;
    OT_OBJECTTRACK_Result trackRes = {0};
    struct timeval start, end;
    ret = SAMPLE_AlgObjectTrackSaveFirstFrameBox(seqInfo, seqBox);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SAMPLE_AlgObjectTrackInitParam");
    ret = SAMPLE_AlgInitTrack(&model, &trackHandle, &memYuvIn);
    OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(ret == TD_SUCCESS, TD_FAILURE, "SAMPLE_AlgInitTrack");

    td_char frame0Path[SAMPLE_ALG_PATH_LEN] = {0};
    td_char *suf = g_configInfo.objectTrackAlgParam.pixFormat == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 ? ".yuv" : ".rgb";
    ret = sprintf_s(frame0Path, SAMPLE_ALG_PATH_LEN - 1, "%s%s%s%s%s", g_configInfo.trackFramePath, "/",
        seqInfo->seqName, "/00000001", suf);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret >= 0, FAIL, "sprintf_s failed");
    ret = SAMPLE_AlgObjectTrack_ReadPreProcessInputData(&g_configInfo.objectTrackAlgParam, &memYuvIn, frame0Path);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SAMPLE_AlgObjectTrack_ReadPreProcessInputData");
    ret = SAMPLE_AlgObjectTrackSetDebugInfo(trackHandle, mainAttr);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SAMPLE_AlgObjectTrackSetDebugInfo");
    ret = SS_OBJECTTRACK_PreExecute(trackHandle, seqBox, &memYuvIn);
    OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SS_OBJECTTRACK_PreExecute");

    td_u32 frmIdx = 1;
    while (g_terminateSignal == TD_FALSE && frmIdx <= SAMPLE_OBJECT_TRACK_IMAGE_MAX_CNT) {
        td_char framePath[SAMPLE_ALG_PATH_LEN] = {0};
        ret = sprintf_s(framePath, SAMPLE_ALG_PATH_LEN - 1, "%s%s%s%s%08u%s", g_configInfo.trackFramePath, "/",
            seqInfo->seqName, "/", frmIdx + 1, suf);
        OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret >= 0, FAIL, "sprintf_s failed");
        if (SAMPLE_CheckFile(framePath) != TD_SUCCESS) {
            goto FAIL;
        }
        ret = SAMPLE_AlgObjectTrack_ReadPreProcessInputData(&g_configInfo.objectTrackAlgParam, &memYuvIn, framePath);
        OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SAMPLE_AlgObjectTrack_ReadPreProcessInputData");
        gettimeofday(&start, TD_NULL);
        ret = SS_OBJECTTRACK_Execute(trackHandle, &memYuvIn, &trackRes);
        OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SS_OBJECTTRACK_Execute");
        gettimeofday(&end, TD_NULL);
        printf("object track execute time: %ldus\n", SAMPLE_ALG_US_PER_S * (end.tv_sec - start.tv_sec) +
            end.tv_usec - start.tv_usec);
        ret = SAMPLE_AlgObjectTrackPostProc(seqInfo, frmIdx, mainAttr, &memYuvIn, &trackRes);
        OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "SAMPLE_AlgObjectTrackPostProc");
        frmIdx++;
    }
FAIL:
    (td_void)SAMPLE_AlgObjectTrackGetTimeCost(trackHandle);
    (td_void)SS_OBJECTTRACK_DestroyTrackHdl(trackHandle);
    (td_void)ss_mpi_sys_mmz_free(memYuvIn.physAddr, memYuvIn.virtAddr);
    (td_void)SS_OBJECTTRACK_Deinit();
    (td_void)ss_mpi_sys_mmz_free(model.physAddr, model.virtAddr);
    return ret;
}

static td_s32 SAMPLE_AlgSysInit()
{
    td_s32 ret;
    (td_void)ss_mpi_sys_exit();

    (td_void)ss_mpi_vb_exit();
    ot_vb_cfg vb_conf = {0};
    ret = ss_mpi_vb_set_cfg(&vb_conf);
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "ss_mpi_vb_set_cfg failed");

    ret = ss_mpi_vb_init();
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "ss_mpi_vb_set_cfg failed");

    ret = ss_mpi_sys_init();
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "ss_mpi_sys_init failed");
    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgSysExit()
{
    td_s32 ret = ss_mpi_sys_exit();
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "ss_mpi_sys_exit failed");

    ret = ss_mpi_vb_exit();
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "ss_mpi_vb_exit failed");

    return TD_SUCCESS;
}

static td_s32 SAMPLE_ParsingParam(const td_char *key, const td_char *value, SAMPLE_ControlMode mode)
{
    if (IsNeedCheck(key, mode) == TD_FAILURE) {
        return TD_SUCCESS;
    }

    ConfigHandler handler = FindHandler(key);
    if (!handler) {
        LOGI("Unknown configuration: %s\n", key);
        return TD_SUCCESS;
    }

    td_s32 ret = handler(value, &g_configInfo);
    if (ret != TD_SUCCESS) {
        LOGE("The value: %s of %s is invalid.\n", value, key);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 SAMPLE_LoadConfig(const td_char *filename, SAMPLE_ControlMode mode)
{
    td_char realPath[PATH_MAX];
    if (realpath(filename, realPath) == TD_NULL && errno != EACCES) {
        LOGE("invalid config file: %s! %s\n", filename, strerror(errno));
        return TD_FAILURE;
    }
    FILE *file = fopen(realPath, "r");
    if (!file) {
        LOGE("Unable to open configuration file");
        return TD_FAILURE;
    }
    td_char line[MAX_LINE_LENGTH];
    td_s32 ret;

    while (fgets(line, sizeof(line), file) != TD_NULL) {
        td_char *trimmed = Trim(line);
        if (trimmed[0] == '#' || strlen(trimmed) == 0) {
            continue;
        }

        td_char *delimiter = strchr(trimmed, '=');
        if (delimiter) {
            *delimiter = '\0';
            const td_char *key = Trim(trimmed);
            const td_char *value = Trim(delimiter + 1);
            ret = SAMPLE_ParsingParam(key, value, mode);
            OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(ret == TD_SUCCESS, FAIL, "Failed to parse the configuration.");
        }
    }
    (td_void)fclose(file);
    return TD_SUCCESS;
FAIL:
    (td_void)fclose(file);
    return TD_FAILURE;
}

static td_s32 SAMPLE_AlgCheckPara(td_s32 argc, td_char *argv[], SAMPLE_MainAttr *mainAttr)
{
    td_s32 idxLen;
    if (argc < SAMPLE_ALG_ARG_SIZE_3) {
        return TD_FAILURE;
    }
    idxLen = (td_s32)strlen(argv[1]);
    if (idxLen != 1 || !isdigit(argv[1][0])) {
        return TD_FAILURE;
    }
    InitConfig();
    mainAttr->mode = atoi(argv[1]);
    switch (mainAttr->mode) {
        case NORM_MODE:
        case DEBUG_MODE:
        case BATCH_MODE:
            return SAMPLE_LoadConfig(argv[SAMPLE_ALG_CONFIG_FILE_INDEX], mainAttr->mode);
        default:
            return TD_FAILURE;
    }
}

static td_void SAMPLE_AlgUsage(td_char *name)
{
    LOGI("usage : %s <index> \n", name);
    LOGI("index:\n");
    LOGI("    (0) object track norm mode                  eg:./sample_object_track 0 ./config.txt\n");
    LOGI("    (1) object track debug mode                 eg:./sample_object_track 1 ./config.txt\n");
    LOGI("    (2) object track batch mode                 eg:./sample_object_track 2 ./config.txt\n");
    LOGI("exit: use ctrl+c to exit\n");
}

static td_s32 SAMPLE_GenerateSeq(OT_IAA_ObjectTrackSeqInfo **seqInfo, OT_OBJECTTRACK_BoxParam **seqBox)
{
    *seqInfo = (OT_IAA_ObjectTrackSeqInfo *)malloc(sizeof(OT_IAA_ObjectTrackSeqInfo));
    *seqBox = (OT_OBJECTTRACK_BoxParam *)malloc(sizeof(OT_OBJECTTRACK_BoxParam));
    if (*seqInfo == TD_NULL || *seqBox == TD_NULL) {
        LOGE("Memory allocation failed\n");
        return TD_FAILURE;
    }
    (td_void)memcpy_s((*seqInfo)->seqName, sizeof(td_char) * SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN,
        "", sizeof(td_char) * SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN);
    (*seqInfo)->height = g_configInfo.objectTrackAlgParam.imgHeight;
    (*seqInfo)->width = g_configInfo.objectTrackAlgParam.imgWidth;
    (*seqBox)->x = g_configInfo.initBoxParam.x;
    (*seqBox)->y = g_configInfo.initBoxParam.y;
    (*seqBox)->width = g_configInfo.initBoxParam.width;
    (*seqBox)->height = g_configInfo.initBoxParam.height;
    return TD_SUCCESS;
}

static td_s32 SAMPLE_CheckLineFormat(const char *line)
{
    char seqName[SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN];
    int width, height, x, y, boxWidth, boxHeight;
    int itemsRead = sscanf_s(line, "%s %d %d %d %d %d %d", seqName, SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN,
        &width, &height, &x, &y, &boxWidth, &boxHeight);
    if (itemsRead == 7) { // 7:seqName, width, height, x, y, boxWidth, boxHeight
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

static td_s32 SAMPLE_GetSeqSize(td_s32 *seqSize)
{
    td_char realPath[PATH_MAX];
    if (realpath(g_configInfo.batchInfoFilePath, realPath) == TD_NULL && errno != EACCES) {
        LOGE("invalid batch info file: %s! %s\n", g_configInfo.batchInfoFilePath, strerror(errno));
        return TD_FAILURE;
    }
    FILE *file = fopen(realPath, "r");
    if (!file) {
        LOGE("Unable to open batch info file");
        return TD_FAILURE;
    }

    td_char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != TD_NULL) {
        line[strcspn(line, "\n")] = 0;
        if (SAMPLE_CheckLineFormat(line) == TD_SUCCESS) {
            (*seqSize)++;
        }
    }
    LOGI("There are %d lines of valid content in th %s\n", *seqSize, realPath);
    (td_void)fclose(file);
    return TD_SUCCESS;
}

static td_s32 SAMPLE_GenerateSeqBatch(OT_IAA_ObjectTrackSeqInfo **seqInfo, OT_OBJECTTRACK_BoxParam **seqBox,
    td_s32 seqSize)
{
    td_char realPath[PATH_MAX];
    if (realpath(g_configInfo.batchInfoFilePath, realPath) == TD_NULL && errno != EACCES) {
        LOGE("invalid batch info file: %s! %s\n", g_configInfo.batchInfoFilePath, strerror(errno));
        return TD_FAILURE;
    }
    FILE *file = fopen(realPath, "r");
    if (!file) {
        LOGE("Unable to open batch info file");
        return TD_FAILURE;
    }
    *seqInfo = (OT_IAA_ObjectTrackSeqInfo *)malloc(seqSize * sizeof(OT_IAA_ObjectTrackSeqInfo));
    *seqBox = (OT_OBJECTTRACK_BoxParam *)malloc(seqSize * sizeof(OT_OBJECTTRACK_BoxParam));
    if (*seqInfo == TD_NULL || *seqBox == TD_NULL) {
        LOGE("Memory allocation failed\n");
        (td_void)fclose(file);
        return TD_FAILURE;
    }

    td_s32 size = 0;
    td_char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && size < seqSize) {
        line[strcspn(line, "\n")] = 0;
        int itemsRead = sscanf_s(line, "%s %d %d %d %d %d %d",
            (*seqInfo)[size].seqName, SAMPLE_OBJECT_TRACK_SEQUENCE_NAME_LEN,
            &(*seqInfo)[size].width,
            &(*seqInfo)[size].height,
            &(*seqBox)[size].x,
            &(*seqBox)[size].y,
            &(*seqBox)[size].width,
            &(*seqBox)[size].height);
        if (itemsRead == 7) { // 7:seqName, width, height, x, y, box_width, box_height
            size++;
        }
    }
    (td_void)fclose(file);
    return TD_SUCCESS;
}

static td_s32 SAMPLE_AlgCase(td_char *argv[], const SAMPLE_MainAttr *mainAttr)
{
    td_s32 ret = TD_SUCCESS;
    g_terminateSignal = TD_FALSE;
    OT_IAA_ObjectTrackSeqInfo *seqInfos = TD_NULL;
    OT_OBJECTTRACK_BoxParam *seqBoxs = TD_NULL;
    td_s32 seqSize = 0;
    switch (mainAttr->mode) {
        case NORM_MODE:
        case DEBUG_MODE:
            seqSize++;
            ret = SAMPLE_GenerateSeq(&seqInfos, &seqBoxs);
            break;
        case BATCH_MODE:
            ret = SAMPLE_GetSeqSize(&seqSize);
            OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "SAMPLE_GetSeqSize failed");
            if (seqSize == 0) {
                return TD_SUCCESS;
            }
            ret = SAMPLE_GenerateSeqBatch(&seqInfos, &seqBoxs, seqSize);
            break;
        default:
            ret = TD_FAILURE;
            break;
    }
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "SAMPLE_AlgCase failed");
    for (td_s32 i = 0; i < seqSize; i++) {
        if (g_terminateSignal == TD_FALSE) {
            (td_void)SAMPLE_AlgObjectTrack(&seqInfos[i], &seqBoxs[i], mainAttr);
        }
    }
    return ret;
}

static td_void SAMPLE_HandleSig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_terminateSignal = TD_TRUE;
    }
}

static td_void SAMPLE_RegisterSigHandler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

td_s32 main(td_s32 argc, td_char *argv[])
{
    td_s32 ret;

    SAMPLE_MainAttr mainAttr = {0};
    if (SAMPLE_AlgCheckPara(argc, argv, &mainAttr) != TD_SUCCESS) {
        SAMPLE_AlgUsage(argv[0]);
        return TD_FAILURE;
    }
    SAMPLE_RegisterSigHandler(SAMPLE_HandleSig);

    ret = SAMPLE_AlgSysInit();
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "SAMPLE_AlgSysInit failed");

    ret = SAMPLE_AlgCase(argv, &mainAttr);
    if (ret != TD_SUCCESS) {
        (td_void)SAMPLE_AlgUsage(argv[0]);
        (td_void)SAMPLE_AlgSysExit();
        return TD_FAILURE;
    }

    ret = SAMPLE_AlgSysExit();
    OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, TD_FAILURE, "SAMPLE_AlgSysExit");

    exit(0);
}
