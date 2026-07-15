#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "securec.h"

typedef struct {
    td_bool is_venc_open;
    td_bool is_vo_open;
} sample_qt_switch_t;

extern int g_sig_flag;

td_s32 sample_vio_start_one_sensor(int sensor_num, sample_vi_cfg *vi_cfg, sample_qt_switch_t *switch_ptr);
void sample_vio_stop_one_sensor(sample_vi_cfg *vi_cfg, sample_qt_switch_t *switch_ptr);