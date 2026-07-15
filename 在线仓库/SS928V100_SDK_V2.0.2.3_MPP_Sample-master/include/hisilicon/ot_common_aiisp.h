/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_COMMON_AIISP_H
#define OT_COMMON_AIISP_H

#include "ot_common_aiv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef ot_aiv_type                  ot_aiisp_type;
typedef ot_aiv_mem_info              ot_aiisp_mem_info;
typedef ot_aiv_model                 ot_aiisp_model;

#define OT_AIISP_TYPE_AIDEHAZE       OT_AIV_TYPE_AIDEHAZE
#define OT_AIISP_TYPE_BUTT           OT_AIV_TYPE_BUTT

#define OT_AIISP_MAX_MODEL_NUM       OT_AIV_MAX_MODEL_NUM
#define OT_AIISP_MAX_DEPTH           OT_AIV_MAX_DEPTH

#define OT_ERR_AIISP_INVALID_PIPE_ID OT_ERR_AIV_INVALID_PIPE_ID
#define OT_ERR_AIISP_ILLEGAL_PARAM   OT_ERR_AIV_ILLEGAL_PARAM
#define OT_ERR_AIISP_NULL_PTR        OT_ERR_AIV_NULL_PTR
#define OT_ERR_AIISP_NOT_SUPPORT     OT_ERR_AIV_NOT_SUPPORT
#define OT_ERR_AIISP_NOT_PERM        OT_ERR_AIV_NOT_PERM
#define OT_ERR_AIISP_NO_MEM          OT_ERR_AIV_NO_MEM
#define OT_ERR_AIISP_NOT_READY       OT_ERR_AIV_NOT_READY

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
