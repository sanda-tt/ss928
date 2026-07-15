/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_AISR_H__
#define __SS_MPI_AISR_H__

#include "ot_common_aisr.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_aisr_init(td_void);
td_s32 ss_mpi_aisr_exit(td_void);

td_s32 ss_mpi_aisr_enable(td_s32 pipe);
td_s32 ss_mpi_aisr_disable(td_s32 pipe);

td_s32 ss_mpi_aisr_set_attr(td_s32 pipe, const ot_aisr_attr *attr);
td_s32 ss_mpi_aisr_get_attr(td_s32 pipe, ot_aisr_attr *attr);

td_s32 ss_mpi_aisr_set_cfg(td_s32 pipe, const ot_aisr_cfg *cfg);
td_s32 ss_mpi_aisr_get_cfg(td_s32 pipe, ot_aisr_cfg *cfg);

td_s32 ss_mpi_aisr_query_status(td_s32 pipe, ot_aisr_status *status);

td_s32 ss_mpi_aisr_load_model(const ot_aisr_model *model, td_s32 *model_id);
td_s32 ss_mpi_aisr_unload_model(td_s32 model_id);

td_s32 ss_mpi_aisr_send_frame(td_s32 pipe, const ot_video_frame_info *frame, td_s32 milli_sec);
td_s32 ss_mpi_aisr_get_frame(td_s32 pipe, ot_video_frame_info *frame, td_s32 milli_sec);
td_s32 ss_mpi_aisr_release_frame(td_s32 pipe, const ot_video_frame_info *frame);


#ifdef __cplusplus
}
#endif

#endif /* __SS_MPI_AISR_H__ */
