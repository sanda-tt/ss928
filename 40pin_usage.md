# SS928 40Pin 引脚占用表

最后更新：2026-07-08

本表只记录当前项目已经占用的 40Pin 信号脚，用于后续增加设备前检查冲突。电源脚、GND、接 VIN/GND/悬空的模块控制脚，以及直接接喇叭的输出端不计入本表。

| 40Pin 索引 | 使用类型 | 板端功能 | 连接设备 | 设备引脚 | 用途/说明 | 来源 |
|---|---|---|---|---|---|---|
| Pin3 | `I2C0` | `I2C0_SDA` | TCA9548A 主端 SDA | BMI270=CH0，左 TM6605=CH1，右 TM6605=CH2 | I2C 数据线，对应 `/dev/i2c-0` | `work/linux_bmi270_backpack/config.ss928_ble.json` |
| Pin5 | `I2C0` | `I2C0_SCL` | TCA9548A 主端 SCL | BMI270=CH0，左 TM6605=CH1，右 TM6605=CH2 | I2C 时钟线，对应 `/dev/i2c-0` | `work/linux_bmi270_backpack/config.ss928_ble.json` |
| Pin8 | `UART4` | `UART4_TXD` | DX-GP21-A GNSS 模块 | `RXD` | GNSS 串口输入，对应 `/dev/ttyAMA4` | `work/dx_gp21_tracker/README.md` |
| Pin10 | `UART4` | `UART4_RXD` | DX-GP21-A GNSS 模块 | `TXD` | GNSS NMEA 串口输出，对应 `/dev/ttyAMA4` | `work/dx_gp21_tracker/README.md` |
| Pin12 | `I2S` | `I2S_BCLK` | MAX98357 I2S 功放 | `BCLK` | I2S 位时钟 | 用户提供的 MAX98357 接线表 |
| Pin38 | `I2S` | `I2S_WS` | MAX98357 I2S 功放 | `LRC` | I2S 左右声道时钟，也叫 `LRCLK`/`WS` | 用户提供的 MAX98357 接线表 |
| Pin40 | `I2S` | `I2S_SD_TX` | MAX98357 I2S 功放 | `DIN` | SS928 音频数据输出，注意不是 `SD_RX` | 用户提供的 MAX98357 接线表 |
| Pin7 | `PWM0` | `PWM0_OUT10_0_P` | SmartBag 左侧光模块 | `PWM` | 小信号 PWM 输入；三级 50% 亮 1 s，四级 80% 脉冲闪烁三次；pinmux `0x102F0110=0x1205` | `work/smartbag_alert_controller/README.md` |
| Pin32 | `PWM0` | `PWM0_OUT1_0_P` | SmartBag 右侧光模块 | `PWM` | 小信号 PWM 输入；三级 50% 亮 1 s，四级 80% 脉冲闪烁三次；pinmux `0x102F01EC=0x1201` | `work/smartbag_alert_controller/README.md` |

## 未计入但容易误会的连接

- BMI270 的 `VCC`、`GND`、`CS/CSB`、`SDO` 分别接 3.3V/GND 或由电源/地决定 I2C 模式与地址，不占用额外 40Pin 信号脚。
- `work/bmi270_i2c_pose/README.md` 中提到 `INT1` 可选接 Pin13、`INT2` 可选接 Pin15；当前 `work/linux_bmi270_backpack` 初版为轮询读取，`INT1/INT2` 先不接，所以 Pin13/Pin15 暂不标记为已用。
- MAX98357 的 `VIN`、`GND`、`SD`、`GAIN`、`OUT1/OUT2` 不计入信号占用表；其中 `SD` 首版接 `VIN` 使能，`GAIN` 接 `VIN` 或悬空，`OUT1/OUT2` 直接接喇叭两端。
