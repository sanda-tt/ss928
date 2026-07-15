/*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description:
 * Author: Software Develop Team
 * Create: 2024-03-21
 */
#ifndef SS_IAA_OBJECT_TRACK_H
#define SS_IAA_OBJECT_TRACK_H

#include <stdio.h>
#include <stdlib.h>
#include "ot_iaa_alg_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define OT_IAA_OBJECT_TRACK_OUTPUT_NUM 2

typedef struct {
    OT_IAA_AlgModelId id;
} OT_IAA_ObjectTrackCalcTmpBufInfo;

typedef struct {
    OT_IAA_AlgData offlineTmplate;
    OT_IAA_AlgData onlineTmplate;
    OT_IAA_AlgData searchRegion;
} OT_IAA_ObjectTrackInput;

typedef struct {
    OT_IAA_AlgModelId id;
    OT_IAA_AlgMemInfo tmpBuf;
    td_bool isAsync;
} OT_IAA_ObjectTrackCtrl;

td_s32 SS_IAA_ObjectTrackLoadModel(const OT_IAA_AlgMemInfo *model, OT_IAA_AlgModelId *id);

td_s32 SS_IAA_ObjectTrackUnloadModel(OT_IAA_AlgModelId id);

td_s32 SS_IAA_ObjectTrackGetTmpBufSize(const OT_IAA_ObjectTrackCalcTmpBufInfo *info, td_u32 *size);

td_s32 SS_IAA_ObjectTrackExecute(const OT_IAA_ObjectTrackInput *input, const OT_IAA_ObjectTrackCtrl *ctrl,
    const OT_IAA_AlgData output[], td_u32 outputNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // SS_IAA_OBJECT_TRACK_H