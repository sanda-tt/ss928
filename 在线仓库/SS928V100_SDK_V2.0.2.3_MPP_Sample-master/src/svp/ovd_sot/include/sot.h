#ifndef SOT_H
#define SOT_H

#include "config.h"

typedef struct {
    SAMPLE_ControlMode mode;
} SAMPLE_MainAttr;

td_s32 AlgInitTrack(OT_IAA_AlgMemInfo *model, td_u32 *trackHandle);
td_s32 AlgObjectTrack_ReadPreProcessInputData(OT_IAA_AlgMemInfo *mem, ot_video_frame_info *frame_info, td_void *virt_addr);
td_s32 AlgObjectTrackSetDebugInfo(td_u32 trackHandle);
td_s32 AlgObjectTrackGetTimeCost(td_u32 trackHandle);

#endif // SOT_H