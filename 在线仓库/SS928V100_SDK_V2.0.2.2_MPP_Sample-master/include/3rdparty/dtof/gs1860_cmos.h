/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Function of imx335_cmos.h
 * Author: ISP SW
 * Create: 2012/06/28
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef __GS1860_CMOS_H_
#define __GS1860_CMOS_H_

#include "ss_mpi_isp.h"
#include "ot_sns_ctrl.h"


#define GS1860_ADDR_BYTE    2
#define GS1860_DATA_BYTE    1
#define CMD_BYTE            1
#define ADDR_BYTE           4
#define DATA_BYTE           4
#define CHK_BYTE            1
#define I2C_BUF_SIZE (CMD_BYTE + ADDR_BYTE + DATA_BYTE + CHK_BYTE)
#define CMD_WRITE           0xF2
#define CMD_READ            0xE2
#define I2C_M_RD            0x0001

#define MAX_LEN             125
#define SECTION_NUM         7

#define GS1860_FULL_LINES_MAX             0xFFFF

#define GS1860_INCREASE_LINES             0 /* make real fps less than stand fps because NVR require */

#define GS1860_VMAX_1M_30FPS_10BIT_LINEAR  (0x1194 + GS1860_INCREASE_LINES)

typedef enum {
    /* 管脚控制寄存器 */
    GS1860_PWM1_OUT15_0_P_PIN = 0,
    GS1860_PWM1_OUT14_0_P_PIN,
    GS1860_PWM1_OUT13_0_P_PIN,
    GS1860_PWM1_OUT12_0_P_PIN,
    GS1860_PWM1_OUT11_0_P_PIN,
    GS1860_PWM1_OUT10_0_P_PIN,
    GS1860_PWM1_OUT9_0_P_PIN,
    GS1860_PWM1_OUT8_0_P_PIN,
    GS1860_PWM1_OUT7_0_P_PIN,
    GS1860_PWM1_OUT6_0_P_PIN,
    GS1860_PWM1_OUT5_0_P_PIN,
    GS1860_PWM1_OUT4_0_P_PIN,
    GS1860_PWM1_OUT3_0_P_PIN,
    GS1860_PWM1_OUT2_0_P_PIN,
    GS1860_PWM1_OUT1_0_P_PIN,
    GS1860_PWM1_OUT0_0_P_PIN,
    GS1860_PWM0_OUT15_0_P_PIN,
    GS1860_PWM0_OUT15_1_P_PIN,
    GS1860_PWM0_OUT14_0_P_PIN,
    GS1860_PWM0_OUT13_0_P_PIN,
    GS1860_PWM0_OUT12_0_P_PIN,
    GS1860_PWM0_OUT11_0_P_PIN,
    GS1860_PWM0_OUT10_0_P_PIN,
    GS1860_PWM0_OUT9_0_P_PIN,
    GS1860_PWM0_OUT8_0_P_PIN,
    GS1860_PWM0_OUT7_0_P_PIN,
    GS1860_PWM0_OUT6_0_P_PIN,
    GS1860_PWM0_OUT5_0_P_PIN,
    GS1860_PWM0_OUT4_0_P_PIN,
    GS1860_PWM0_OUT3_0_P_PIN,
    GS1860_PWM0_OUT2_0_P_PIN,
    GS1860_PWM0_OUT1_0_P_PIN,
    GS1860_PWM0_OUT0_0_P_PIN,

    /* 时钟软复位控制寄存器 */
    GS1860_PWM0_CKEN,
    GS1860_PWM1_CKEN,

    /* PWM(x)周期配置寄存器 */
    GS1860_PWM1_OUT15_PERIOD_CFG,
    GS1860_PWM1_OUT14_PERIOD_CFG,
    GS1860_PWM1_OUT13_PERIOD_CFG,
    GS1860_PWM1_OUT12_PERIOD_CFG,
    GS1860_PWM1_OUT11_PERIOD_CFG,
    GS1860_PWM1_OUT10_PERIOD_CFG,
    GS1860_PWM1_OUT9_PERIOD_CFG,
    GS1860_PWM1_OUT8_PERIOD_CFG,
    GS1860_PWM1_OUT7_PERIOD_CFG,
    GS1860_PWM1_OUT6_PERIOD_CFG,
    GS1860_PWM1_OUT5_PERIOD_CFG,
    GS1860_PWM1_OUT4_PERIOD_CFG,
    GS1860_PWM1_OUT3_PERIOD_CFG,
    GS1860_PWM1_OUT2_PERIOD_CFG,
    GS1860_PWM1_OUT1_PERIOD_CFG,
    GS1860_PWM1_OUT0_PERIOD_CFG,

    GS1860_PWM0_OUT15_PERIOD_CFG,
    GS1860_PWM0_OUT14_PERIOD_CFG,
    GS1860_PWM0_OUT13_PERIOD_CFG,
    GS1860_PWM0_OUT12_PERIOD_CFG,
    GS1860_PWM0_OUT11_PERIOD_CFG,
    GS1860_PWM0_OUT10_PERIOD_CFG,
    GS1860_PWM0_OUT9_PERIOD_CFG,
    GS1860_PWM0_OUT8_PERIOD_CFG,
    GS1860_PWM0_OUT7_PERIOD_CFG,
    GS1860_PWM0_OUT6_PERIOD_CFG,
    GS1860_PWM0_OUT5_PERIOD_CFG,
    GS1860_PWM0_OUT4_PERIOD_CFG,
    GS1860_PWM0_OUT3_PERIOD_CFG,
    GS1860_PWM0_OUT2_PERIOD_CFG,
    GS1860_PWM0_OUT1_PERIOD_CFG,
    GS1860_PWM0_OUT0_PERIOD_CFG,

    /* PWM(x)占空比配置寄存器 */
    GS1860_PWM1_OUT15_DUTY0_CFG,
    GS1860_PWM1_OUT14_DUTY0_CFG,
    GS1860_PWM1_OUT13_DUTY0_CFG,
    GS1860_PWM1_OUT12_DUTY0_CFG,
    GS1860_PWM1_OUT11_DUTY0_CFG,
    GS1860_PWM1_OUT10_DUTY0_CFG,
    GS1860_PWM1_OUT9_DUTY0_CFG,
    GS1860_PWM1_OUT8_DUTY0_CFG,
    GS1860_PWM1_OUT7_DUTY0_CFG,
    GS1860_PWM1_OUT6_DUTY0_CFG,
    GS1860_PWM1_OUT5_DUTY0_CFG,
    GS1860_PWM1_OUT4_DUTY0_CFG,
    GS1860_PWM1_OUT3_DUTY0_CFG,
    GS1860_PWM1_OUT2_DUTY0_CFG,
    GS1860_PWM1_OUT1_DUTY0_CFG,
    GS1860_PWM1_OUT0_DUTY0_CFG,

    GS1860_PWM0_OUT15_DUTY0_CFG,
    GS1860_PWM0_OUT14_DUTY0_CFG,
    GS1860_PWM0_OUT13_DUTY0_CFG,
    GS1860_PWM0_OUT12_DUTY0_CFG,
    GS1860_PWM0_OUT11_DUTY0_CFG,
    GS1860_PWM0_OUT10_DUTY0_CFG,
    GS1860_PWM0_OUT9_DUTY0_CFG,
    GS1860_PWM0_OUT8_DUTY0_CFG,
    GS1860_PWM0_OUT7_DUTY0_CFG,
    GS1860_PWM0_OUT6_DUTY0_CFG,
    GS1860_PWM0_OUT5_DUTY0_CFG,
    GS1860_PWM0_OUT4_DUTY0_CFG,
    GS1860_PWM0_OUT3_DUTY0_CFG,
    GS1860_PWM0_OUT2_DUTY0_CFG,
    GS1860_PWM0_OUT1_DUTY0_CFG,
    GS1860_PWM0_OUT0_DUTY0_CFG,

    /* PWM(x)工作模式配置寄存器 */
    GS1860_PWM1_OUT15_CTRL,
    GS1860_PWM1_OUT14_CTRL,
    GS1860_PWM1_OUT13_CTRL,
    GS1860_PWM1_OUT12_CTRL,
    GS1860_PWM1_OUT11_CTRL,
    GS1860_PWM1_OUT10_CTRL,
    GS1860_PWM1_OUT9_CTRL,
    GS1860_PWM1_OUT8_CTRL,
    GS1860_PWM1_OUT7_CTRL,
    GS1860_PWM1_OUT6_CTRL,
    GS1860_PWM1_OUT5_CTRL,
    GS1860_PWM1_OUT4_CTRL,
    GS1860_PWM1_OUT3_CTRL,
    GS1860_PWM1_OUT2_CTRL,
    GS1860_PWM1_OUT1_CTRL,
    GS1860_PWM1_OUT0_CTRL,

    GS1860_PWM0_OUT15_CTRL,
    GS1860_PWM0_OUT14_CTRL,
    GS1860_PWM0_OUT13_CTRL,
    GS1860_PWM0_OUT12_CTRL,
    GS1860_PWM0_OUT11_CTRL,
    GS1860_PWM0_OUT10_CTRL,
    GS1860_PWM0_OUT9_CTRL,
    GS1860_PWM0_OUT8_CTRL,
    GS1860_PWM0_OUT7_CTRL,
    GS1860_PWM0_OUT6_CTRL,
    GS1860_PWM0_OUT5_CTRL,
    GS1860_PWM0_OUT4_CTRL,
    GS1860_PWM0_OUT3_CTRL,
    GS1860_PWM0_OUT2_CTRL,
    GS1860_PWM0_OUT1_CTRL,
    GS1860_PWM0_OUT0_CTRL,
    GS1860_PWM_MAX,
} gs1860_pwm_addr;

typedef enum {
    GS1860_5M_30FPS_12BIT_LINEAR_MODE = 0,
    GS1860_4M_30FPS_10BIT_WDR_MODE,
    GS1860_MODE_BUTT
} GS1860_RES_MODE_E;

typedef struct {
    td_u32      ver_lines;
    td_u32      max_ver_lines;
    td_float    max_fps;
    td_float    min_fps;
    td_u32      width;
    td_u32      height;
    td_u8       sns_mode;
    ot_wdr_mode wdr_mode;
    const char *mode_name;
} gs1860_video_mode_tbl;

typedef struct {
    td_u32 addr;
    td_u32 data;
} gs1860_register;

typedef struct {
    td_char section[MAX_LEN];
    gs1860_register reg[MAX_LEN];
    td_s32 register_len;
} gs1860_config;

ot_isp_sns_state *gs1860_get_ctx(ot_vi_pipe vi_pipe);
ot_isp_sns_commbus *gs1860_get_bus_info(ot_vi_pipe vi_pipe);

td_void gs1860_init(ot_vi_pipe vi_pipe);
td_void gs1860_exit(ot_vi_pipe vi_pipe);


void power_off_control(ot_vi_pipe vi_pipe);
void gs1860_1000ps_config(ot_vi_pipe vi_pipe);
void gs1860_500ps_config(ot_vi_pipe vi_pipe);

int gs1860_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data);
int gs1860_read_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 *read_data);

td_s32 gs1860_read_eeprom(ot_vi_pipe vi_pipe, td_u8 *read_data, td_u32 read_data_len);

float gs1860_tsensor_temperature(ot_vi_pipe vi_pipe);
td_s32 gs1860_read_ini_file(char* file_path);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif  /* __GS1860_CMOS_H_ */
