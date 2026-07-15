/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */
#ifndef __SS_MPI_MOTIONFUSION_H__
#define __SS_MPI_MOTIONFUSION_H__

#include "ot_type.h"
#include "ot_common_motionfusion.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOTIONFUSION_DEVICE_ID_0 0
#define MOTIONFUSION_DEVICE_ID_1 1

td_s32 ss_mpi_mfusion_set_attr(const td_u32 fusion_id, const ot_mfusion_attr *mfusion_attr);
td_s32 ss_mpi_mfusion_get_attr(const td_u32 fusion_id, ot_mfusion_attr *mfusion_attr);

td_s32 ss_mpi_mfusion_set_gyro_drift(const td_u32 fusion_id, const ot_mfusion_drift *drift);
td_s32 ss_mpi_mfusion_get_gyro_drift(const td_u32 fusion_id, ot_mfusion_drift *drift);

td_s32 ss_mpi_mfusion_set_gyro_six_side_calibration(const td_u32 fusion_id,
    const ot_mfusion_six_side_calibration *six_side_calibration);
td_s32 ss_mpi_mfusion_get_gyro_six_side_calibration(const td_u32 fusion_id,
    ot_mfusion_six_side_calibration *six_side_calibration);

td_s32 ss_mpi_mfusion_set_gyro_temperature_drift(const td_u32 fusion_id,
    const ot_mfusion_temperature_drift *temperature_drift);
td_s32 ss_mpi_mfusion_get_gyro_temperature_drift(const td_u32 fusion_id,
    ot_mfusion_temperature_drift *temperature_drift);

td_s32 ss_mpi_mfusion_set_gyro_online_temperature_drift(const td_u32 fusion_id,
    const ot_mfusion_temperature_drift *temperature_drift);
td_s32 ss_mpi_mfusion_get_gyro_online_temperature_drift(const td_u32 fusion_id,
    ot_mfusion_temperature_drift *temperature_drift);

td_s32 ss_mpi_mfusion_set_gyro_online_drift(const td_u32 fusion_id, const ot_mfusion_drift *drift);
td_s32 ss_mpi_mfusion_get_gyro_online_drift(const td_u32 fusion_id, ot_mfusion_drift *drift);

td_s32 ss_mpi_mfusion_init_rotation_compensation(const td_u32 fusion_id,
    const ot_mfusion_rotation_cfg *cfg);
td_s32 ss_mpi_mfusion_deinit_rotation_compensation(const td_u32 fusion_id);
td_s32 ss_mpi_mfusion_get_rotation_compensation(const td_u32 fusion_id,
    ot_mfusion_rotation_compensation *compensation);
td_s32 ss_mpi_mfusion_set_rotation_compensation(const td_u32 fusion_id,
    const ot_mfusion_rotation_compensation *compensation);
td_s32 ss_mpi_mfusion_send_quaternion(const td_u32 fusion_id, ot_mfusion_quaternion_buf *quaternion_buf);
td_s32 ss_mpi_mfusion_get_quaternion(const td_u32 fusion_id, td_u64 begin_pts, td_u64 end_pts,
    ot_mfusion_quaternion_buf *quaternion_buf);
td_s32 ss_mpi_mfusion_send_rotation_compensation_data(const td_u32 fusion_id, ot_mfusion_position_data *position_data);
td_s32 ss_mpi_mfusion_get_rotation_compensation_data(const td_u32 fusion_id, td_u64 begin_pts, td_u64 end_pts,
    ot_mfusion_position_data *position_data);

td_s32 ss_mpi_mfusion_set_acc_six_side_calibration(const td_u32 fusion_id,
    const ot_mfusion_six_side_calibration *six_side_calibration);
td_s32 ss_mpi_mfusion_get_acc_six_side_calibration(const td_u32 fusion_id,
    ot_mfusion_six_side_calibration *six_side_calibration);

td_s32 ss_mpi_mfusion_set_acc_drift(const td_u32 fusion_id, const ot_mfusion_drift *drift);
td_s32 ss_mpi_mfusion_get_acc_drift(const td_u32 fusion_id, ot_mfusion_drift *drift);

td_s32 ss_mpi_mfusion_get_imu_data(const td_u32 fusion_id,
    const td_u64 begin_pts, const td_u64 end_pts, ot_mfusion_gyro_data *gyro_data, ot_mfusion_acc_data *acc_data);

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __SS_MPI_MONTIONFUSION_H__ */
