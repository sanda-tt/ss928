 /*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: object track api module.
 * Author: Software Develop Team
 * Create: 2024-03-10
 */

#ifndef SS_OBJECT_TRACK_H
#define SS_OBJECT_TRACK_H

#include "ot_type.h"
#include "ot_object_track_define.h"
#include "ss_iaa_alg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/**
 * @brief Initializes object tracking.
 *        Multi-thread/task concurrent invoking is not supported.
 *        Repeated invocation is not supported.
 *        This interface is invoked after the SYS module of the MPP is initialized.
 * @param model [IN]   The value cannot be empty. Ensure that the address content is a tracing model.
 *
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_Init(const OT_IAA_AlgMemInfo *model);

/**
 * @brief Deinitializes object tracking.
 *        Multi-thread/task concurrent invoking is not supported.
 *        Repeated invocation is not supported.
 *        Called after SS_OBJECTTRACK_Init
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_Deinit(td_void);

/**
 * @brief Creates a object tracing handle and allocates tracing auxiliary memory.
 *        Multi-thread/task concurrent invoking is not supported.
 *        Multiple invoking is supported. Different handle numbers are returned for subsequent tracing.
 *        A maximum of OT_OBJECT_TRACK_MAX_HANDLE_CNT handles are supported.
 *        The interface is invoked between SS_OBJECTTRACK_Init and SS_OBJECTTRACK_Deinit.
 * @param algParam [IN]  Algorithm parameter, which cannot be empty.
 * @param trackHandle [OUT]  Handle ID, which cannot be null.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_CreateTrackHdl(const OT_OBJECTTRACK_AlgParam *algParam, td_u32 *trackHandle);

/**
 * @brief Destroys the object trace handle and releases the trace auxiliary memory.
 *        Multi-thread/task concurrent invoking is not supported.
 *        This interface is invoked after SS_OBJECTTRACK_CreateTrackHdl and the handle must have been created.
 * @param trackHandle [IN]  Handle ID.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_DestroyTrackHdl(td_u32 trackHandle);

/**
 * @brief Pre-execution of object tracking. Generally, this operation is performed only once.
 *        Repeated invoking is supported.
 *        Multi-thread/task concurrent invoking is supported.
 *        This interface is invoked after SS_OBJECTTRACK_CreateTrackHdl to initialize the online and offline templates.
 * @param trackHandle [IN]  Handle ID.
 * @param iniBox [IN]  Initial target frame.
 * @param frameIn [IN]  Input Image.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_PreExecute(td_u32 trackHandle, const OT_OBJECTTRACK_BoxParam *iniBox,
    const OT_IAA_AlgMemInfo *frameIn);

/**
 * @brief Tracing Execution Operations.
 *        Multi-thread/task concurrent invoking is supported.
 *        required distance SS_OBJECTTRACK_PreExecute
 * @param trackHandle [IN]  Handle ID.
 * @param frameIn [IN]  Input Image.
 * @param trackRes [OUT]  Trace results.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_Execute(td_u32 trackHandle, const OT_IAA_AlgMemInfo *frameIn, OT_OBJECTTRACK_Result *trackRes);

// dfx
/**
 * @brief Sets the trace handle DFX information.
 *        The trace handle must have been created.
 * @param trackHandle [IN]  Handle ID.
 * @param dfxInfo [IN]  dfxInfo information.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_SetHdlDfxInfo(td_u32 trackHandle, const OT_OBJECTTRACK_DfxInfo *dfxInfo);

/**
 * @brief Obtains the DFX information of the trace handle.
 *        The trace handle must have been created.
 * @param trackHandle [IN]  Handle ID.
 * @param dfxInfo [OUT]  dfxInfo information.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_GetHdlDfxInfo(td_u32 trackHandle, OT_OBJECTTRACK_DfxInfo *dfxInfo);

/**
 * @brief Obtains the tracing handle algorithm parameters.
 *        The trace handle must have been created.
 * @param trackHandle [IN]  Handle ID.
 * @param algParam [OUT]  Algorithm parameter.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_QueryHdlAlgParam(td_u32 trackHandle, OT_OBJECTTRACK_AlgParam *algParam);

/**
 * @brief Querying the Processing Duration of Trace Handles in Each Phase.
 *        The trace handle must have been created.
 * @param trackHandle [IN]  Handle ID.
 * @param timeCost [OUT]  Tracing duration.
 * @retval The value 0 indicates success, and other values indicate failure.
 */
td_s32 SS_OBJECTTRACK_QueryTimeCost(td_u32 trackHandle, OT_OBJECTTRACK_DfxTimeCost *timeCost);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef SS_OBJECT_TRACK_H */