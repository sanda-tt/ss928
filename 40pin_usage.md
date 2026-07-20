# SS928 40Pin 引脚占用表

最后更新：2026-07-20

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

## EULER_40PEXP 扩展板使用说明

`Pin3`、`Pin12` 等是主板接入扩展板的 40Pin 编号；扩展板外侧按功能丝印接线，不能按外侧排针的序号猜测。下表是当前项目使用的扩展板侧映射。

| 用途 | 原 40Pin | 扩展板接口/丝印 | 模块连接 | 当前状态 |
|---|---|---|---|---|
| I2C0 数据 | Pin3 `I2C0_SDA` | `J6`（OLED）-3，`I2C0_SDA` | TCA9548A `SDA` | 已占用 |
| I2C0 时钟 | Pin5 `I2C0_SCL` | `J6`（OLED）-4，`I2C0_SCL` | TCA9548A `SCL` | 已占用 |
| I2C0 电源/地 | Pin1/17、GND | `J6`-1 `3V3`、`J6`-2 `GND` | TCA9548A `VCC`、`GND` | 已占用 |
| GNSS 发送 | Pin8 `UART4_TXD` | `J8`-4，`T4` | DX-GP21-A `RXD` | 已占用 |
| GNSS 接收 | Pin10 `UART4_RXD` | `J8`-6，`R4` | DX-GP21-A `TXD` | 已占用 |
| I2S 位时钟 | Pin12 `I2S_BCLK` | `J7`-4，`TDO` | MAX98357 `BCLK` | 已占用 |
| I2S 声道时钟 | Pin38 `I2S_WS` | `J7`-7，JTAG 组 `CK` | MAX98357 `LRC` / `WS` | 已占用 |
| I2S 音频数据 | Pin40 `I2S_SD_TX` | `J7`-1，JTAG 组 `RST` / `TRSTN` | MAX98357 `DIN` | 已占用 |
| 左灯 PWM | Pin7 `PWM0_OUT10_0_P` | `J7`-3，`TDI` | 左灯模块 `PWM` / `SIG` | 已占用 |
| 右灯 PWM | Pin32 `PWM0_OUT1_0_P` | `J5`（舵机2）-4，`PWM0_1` | 右灯模块 `PWM` / `SIG` | 已占用 |

- `J7` 的 `CK`、`RST` 必须使用与 `TDO`、`TDI`、`TMS` 同一组 JTAG 排针的丝印；不要误接 SPI 区域的 `CK` 或 LCD 的 `RST`。
- MAX98357 只需 `BCLK`、`LRC/WS`、`DIN` 三根 I2S 信号；`I2S_MCLK` 不需要。`VIN` 可先接 3.3V，`GND` 必须共地，`SD` 接 3.3V 使能，`GAIN` 可悬空。
- PWM 脚只输出控制信号。灯模块按其额定电压独立供电，并与 SS928 GND 共地；不得由 PWM 脚直接带动灯负载。

## TCA9548A 多路 I2C

TCA9548A 将一组 I2C0 主线分成 8 个相互隔离的下游通道。当前分配为：`CH0`=BMI270、`CH1`=左 TM6605、`CH2`=右 TM6605；其余通道先不接。

```text
SS928 J6 SDA/SCL
       │
       └── TCA9548A（地址 0x70）
           ├── CH0 ── BMI270（0x68）
           ├── CH1 ── 左 TM6605（0x2D）
           └── CH2 ── 右 TM6605（0x2D）
```

接线与规则：

- TCA9548A 主端：`SDA` 接 J6-3，`SCL` 接 J6-4，`VCC` 接 3.3V，`GND` 共地；`A0/A1/A2` 全接 GND，地址为 `0x70`；`RESET` 保持高电平（接 3.3V）。
- 每个下游设备只接到自己的 `SDx/SCx` 通道。BMI270 的 `SDA/SCL` 接 CH0，左右 TM6605 的 `SDA/SCL` 分别接 CH1/CH2。
- 两块 TM6605 的地址都为 `0x2D`，因此绝不能直接并联在同一个 TCA 通道；读取/写入前，程序必须先选择对应通道。
- 推荐让 TCA9548A 的逻辑侧和 I2C 上拉保持 3.3V。TM6605 可以单独使用 5V 电源，但其 I2C 信号必须确认支持 3.3V；若模块板把 SDA/SCL 固定上拉到 5V，必须在主线与该 5V I2C 支路之间增加双向 I2C 电平转换器，不能直接接到 SS928。
- 板端 pinmux：`bspmm 0x102F013c 0x2031`（SDA）和 `bspmm 0x102F0140 0x2031`（SCL）。

## 未计入但容易误会的连接

- BMI270 的 `VCC`、`GND`、`CS/CSB`、`SDO` 分别接 3.3V/GND 或由电源/地决定 I2C 模式与地址，不占用额外 40Pin 信号脚。
- `work/bmi270_i2c_pose/README.md` 中提到 `INT1` 可选接 Pin13、`INT2` 可选接 Pin15；当前 `work/linux_bmi270_backpack` 初版为轮询读取，`INT1/INT2` 先不接，所以 Pin13/Pin15 暂不标记为已用。
- MAX98357 的 `VIN`、`GND`、`SD`、`GAIN`、`OUT1/OUT2` 不计入信号占用表；其中 `SD` 首版接 `VIN` 使能，`GAIN` 接 `VIN` 或悬空，`OUT1/OUT2` 直接接喇叭两端。
