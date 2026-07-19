# TM6605 双 LRA 触觉预警（2026-07-19）

## 目标

将 SmartBag 原有四路 PWM 震动模块替换为左、右各一块 TM6605 LRA 触觉驱动模块，并保持原有四级预警入口。

## 硬件事实与接线

- TM6605 是 I2C 控制、固定 7-bit 地址 `0x2D` 的 LRA 驱动器，不接受 GPIO PWM 作为震动强度输入。
- 两块 TM6605 地址相同，必须通过 TCA9548A I2C 多路复用器隔离；40Pin I2C0 的 Pin3/SDA、Pin5/SCL 接 TCA9548A，通道 0 给左侧、通道 1 给右侧。
- 两块 TM6605 与 TCA9548A 使用 5V；SS928 Pin3/5 至 TCA9548A 主线间加入双向 I2C 电平转换器（3.3V/5V），所有 GND 相连。不得让模块的 5V I2C 上拉直接接入 SS928。
- 旧震动 PWM 引脚 Pin7、Pin32、Pin35、Pin37 不再使用。

## 最终行为

- 1/2 级不播放触觉效果。
- 3 级播放 TM6605 effect 15（三次中等时长效果），总时长约 2 秒，定义为 80% 级别。
- 4 级播放 TM6605 effect 14（三次强脉冲），定义为 100% 级别。
- TM6605 自动在 LRA 共振频率工作；“80%/100%”是触觉效果级别，不能映射为旧 GPIO PWM 占空比。

## 关键文件

- `work/smartbag_alert_controller/tm6605_haptics.py`
- `work/smartbag_alert_controller/smartbag_alert_controller.py`
- `work/smartbag_alert_controller/start_ss928_smartbag_alert.sh`
- `work/smartbag_alert_controller/smartbag.env.example`

## 验证与未验证项

- 已在 Windows 完成 `py_compile`、18 项单元测试、以及 `--dry-run` I2C 命令序列验证。
- 未连接 SS928 和真实模块；待在板端确认 `/dev/i2c-0`、TCA9548A 地址/通道、Pin3/5 pinmux、供电和左右震感。

## 不要重复的错误路线

- 不要把两个固定地址 `0x2D` 的 TM6605 直接并到同一 I2C 总线上。
- 不要把 LRA 或 TM6605 误接到旧 PWM 输出，也不要将 5V I2C 上拉直接接入 3.3V SS928 I2C 引脚。
