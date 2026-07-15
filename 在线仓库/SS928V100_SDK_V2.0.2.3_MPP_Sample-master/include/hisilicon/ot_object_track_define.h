 /*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: object track definition module.
 * Author: Software Develop Team
 * Create: 2024-03-10
 */

#ifndef OT_OBJECT_TRACK_DEFINE_H
#define OT_OBJECT_TRACK_DEFINE_H

#include "ot_type.h"
#include "ss_mpi_vgs.h"
#include "ss_iaa_alg.h"
#include "ot_common_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* define error code */
#define OT_OBJECTTRACK_EINTER         0x81000001 /* Internal error */
#define OT_OBJECTTRACK_EINVAL         0x81000002 /* illegal parameter */
#define OT_OBJECTTRACK_ENOINIT        0x81000003 /* no init */
#define OT_OBJECTTRACK_ENORES         0x81000004 /* no resource */
#define OT_OBJECTTRACK_EEXIST         0x81000005 /* no resource */
#define OT_OBJECTTRACK_EINITIALIZED   0x81000006 /* already initialized */
#define OT_OBJECTTRACK_ECREATED       0x81000007 /* already created */
#define OT_OBJECTTRACK_EDESTOYED      0x81000008 /* already destoryed */
#define OT_OBJECTTRACK_EBUSY          0x81000009 /* handle busy */
#define OT_OBJECTTRACK_ENULLPTR       0x8100000A /* null ptr */

#define OT_OBJECT_TRACK_NPU_OUTPUT_NUM 2
#define OT_OBJECT_TRACK_MIN_MARGIN 10 /* Minimum side length of the trace input/output box */
#define OT_OBJECT_TRACK_MAX_HANDLE_CNT 4 /* Maximum number of traced handles, every thread is one handle */
#define OT_OBJECT_TRACK_MAX_PATH_LEN 128

typedef enum {
    OT_OBJECT_TRACK_MODEL_TINY = 0X0,
    OT_OBJECT_TRACK_MODEL_SMALL,
    OT_OBJECT_TRACK_MODEL_BASE,
    OT_OBJECT_TRACK_MODEL_BUTT,
} OT_OBJECTTRACK_ModelType;

/* u32 input box */
typedef struct {
    td_s32 x; /* x > 0, x + width < imageWidth */
    td_s32 y; /* y > 0, y + height < imageHeight */
    td_s32 width; /* The minimum value is 10. */
    td_s32 height; /* The minimum value is 10. */
} OT_OBJECTTRACK_BoxParam;

typedef struct {
    OT_OBJECTTRACK_BoxParam boxOut;
    td_float score; /* Confidence range of the target prediction box: [0, 1]. */
} OT_OBJECTTRACK_Result;

/* object track alg param */
typedef struct {
    td_float onlineUpdateThresh; /* Value range: [0, 1). The default value 0.9 is recommended. */
    td_float templateFactor; /* Template magnification factor. Value range: [1.0, 6.0]. */
    td_float searchFactor; /* Magnification factor of the search area. Value range: [1.0, 6.0]. */
    td_u32 imgHeight; /* Value range: [100, 1080]. Must be 2-pixel aligned */
    td_u32 imgWidth; /* Value range: [100, 1920]. Must be 2-pixel aligned */
    td_u32 onlineUpdateInter; /* Value > 0 */
    ot_pixel_format pixFormat; /* Value: OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 or OT_PIXEL_FORMAT_RGB_888 */
    OT_OBJECTTRACK_ModelType modelType; /* Model type enumeration, which corresponds to the actual model. */
} OT_OBJECTTRACK_AlgParam;

/* dfx info */
typedef struct {
    td_bool enableDump;
    td_char dumpPath[OT_OBJECT_TRACK_MAX_PATH_LEN]; /* Dump path. Relative paths are supported. The path must exist. */
} OT_OBJECTTRACK_DfxInfo;

/* dfx time info */
typedef struct {
    td_u64 aveTimeCost; /* Average duration, in us. */
    td_u64 maxTimeCost; /* Maximum duration, in us. */
    td_u64 minTimeCost; /* Minimum duration, in us. */
} OT_OBJECTTRACK_DfxTimeInfo;

/* dfx time cost */
typedef struct {
    OT_OBJECTTRACK_DfxTimeInfo yuv2Rgb; /* Time required for converting YUV images to RGB images. */
    OT_OBJECTTRACK_DfxTimeInfo createSea; /* Time required for generating a search area. */
    OT_OBJECTTRACK_DfxTimeInfo postProc; /* Total time required for post-processing */
    OT_OBJECTTRACK_DfxTimeInfo npuProc; /* Total NPU inference duration */
    OT_OBJECTTRACK_DfxTimeInfo totalProc; /* Total Time Required */
} OT_OBJECTTRACK_DfxTimeCost;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef OT_IAA_OBJECT_TRACK_DEFINE_H */