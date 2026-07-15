/*
 * Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef OT_PWM_H
#define OT_PWM_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    unsigned char pwm_num;  /* 0:PWM0,1:PWM1,2:PWM2,3:PWMII0,4:PWMII1,5:PWMII2 */
    unsigned int  duty;
    unsigned int  period;
    unsigned char enable;
} pwm_data;

typedef enum {
    IOC_PWM_ISP_CMD_WRITE = 0,
    IOC_PWM_ISP_CMD_READ,
    IOC_PWM_ISP_BUTT,
} ioc_pwm_isp;
#define PWM_IOC_MAGIC    'p'
#define PWM_CMD_WRITE  _IOW(PWM_IOC_MAGIC, IOC_PWM_ISP_CMD_WRITE, pwm_data)
#define PWM_CMD_READ  _IOR(PWM_IOC_MAGIC, IOC_PWM_ISP_CMD_READ, pwm_data)

int pwm_drv_write(unsigned char pwm_num, unsigned int duty, unsigned int period, unsigned char enable);
int pwm_init(void);
void pwm_exit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* OT_PWM_H */
