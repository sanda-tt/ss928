/*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description:
 * Author: Software Develop Team
 * Create: 2024-02-25
 */
#ifndef SS_IAA_ALG_H
#define SS_IAA_ALG_H

#include <stdio.h>
#include <stdlib.h>
#include "ot_type.h"
#include "ot_iaa_alg_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

td_s32 SS_IAA_AlgInit(td_void);
td_s32 SS_IAA_AlgDeinit(td_void);

td_s32 SS_IAA_AlgSync(td_void);

// dfx
td_s32 SS_IAA_DfxSetSaveFilePath(const td_char *path);

td_s32 SS_IAA_DfxSetCtrlInfo(const OT_IAA_DfxCtrl *ctrl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // SS_IAA_ALG_H