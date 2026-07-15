/*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description:
 * Author: Software Develop Team
 * Create: 2024-5-8
 */
#ifndef SS_IAA_FEATURE_OPERATOR_H
#define SS_IAA_FEATURE_OPERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include "ot_iaa_alg_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define OT_IAA_FO_INPUT_NUM 5     /* intput_img, match_last_desc, warp_roi, match_nn_thresh, match_ratio_test */
#define OT_IAA_FO_OUTPUT_NUM 8    /* match_max_indices, prematch_normalize_desc, topk_pts, match_keep */

typedef struct {
    OT_IAA_AlgData inputImg;
    OT_IAA_AlgData matchLastDesc;
    OT_IAA_AlgData warpRoi;
    OT_IAA_AlgData matchNnThresh;
    OT_IAA_AlgData matchRatioTest;
} OT_IAA_FoInput;

typedef struct {
    OT_IAA_AlgData topKVal;
    OT_IAA_AlgData procPts;
    OT_IAA_AlgData topKPts;
    OT_IAA_AlgData warpOut;
    OT_IAA_AlgData oriDesc;
    OT_IAA_AlgData normalizeDesc;
    OT_IAA_AlgData matchMaxIndices;
    OT_IAA_AlgData matchKeep;
} OT_IAA_FoOutput;

typedef struct {
    OT_IAA_AlgModelId id;
    OT_IAA_AlgMemInfo tmpBuf;
    td_bool isAsync;
} OT_IAA_FO_Ctrl;

td_s32 SS_IAA_FoLoadModel(const OT_IAA_AlgMemInfo *model, OT_IAA_AlgModelId *id);

td_s32 SS_IAA_FoUnloadModel(OT_IAA_AlgModelId id);

td_s32 SS_IAA_FoGetTmpBufSize(OT_IAA_AlgModelId id, td_u32 *size);

td_s32 SS_IAA_FoExecute(const OT_IAA_FoInput *input, const OT_IAA_FO_Ctrl *ctrl, const OT_IAA_FoOutput *output);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // SS_IAA_FEATURE_OPERATOR_H