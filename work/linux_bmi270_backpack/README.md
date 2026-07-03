# BMI270 背包姿态检测与速度估算程序

这个目录是一套板端 Linux 控制程序示例，目标是：

- 通过 Linux IIO 读取 BMI270 的三轴加速度和三轴角速度。
- 估算背包姿态：roll / pitch / yaw。
- 估算短时间速度 `speed_mps`，用于运动趋势判断。
- 姿态异常时输出特殊报警信号：串口/终端 `ALERT,...`，并可选脉冲写入文件或 GPIO value。
- 通过蓝牙 BLE Nordic UART Service 把数据发到手机。

> 重要：仅靠 BMI270 六轴积分出来的“速度”会漂移，适合短时间趋势、是否剧烈运动、是否超过粗略阈值；如果要可靠的行走/骑行/车辆速度，建议后续加 GPS、轮速、UWB 或手机定位进行融合。

## 文件

- `bmi270_backpack.py`：主程序。
- `config.example.json`：可调阈值和输出配置。
- `config.ss928_ble.json`：SS928 40Pin + 微信小程序 BLE 联调配置，按旧 BMI270 接线固定为 `/dev/i2c-0`、`0x68`。
- `start_ss928_ble.sh`：SS928 上一键启动 I2C0 pinmux、ws73 BLE 栈和本程序 BLE 外设。
- `bmi270-backpack.service`：systemd 自启动模板。

## 板端依赖

假设开发板已经有 BMI270 的 Linux IIO 驱动，并能看到 `/sys/bus/iio/devices/iio:deviceX`。

常见依赖：

```bash
sudo apt install python3 python3-dbus python3-gi bluez
sudo systemctl enable --now bluetooth
bluetoothctl power on
```

如果你的系统不是 Debian/Ubuntu，把包名换成发行版对应的 BlueZ、python dbus、python gi 包即可。

## SS928 旧接线 + 微信小程序 BLE 初版

如果沿用 `work/bmi270_i2c_pose` 的 BMI270 接线，第一版固定按下面方式接，不要混用其他板卡示例里的 I2C1：

| BMI270 模块 | SS928 40Pin | 说明 |
| --- | --- | --- |
| VCC | Pin1 或 Pin17 | 3.3V，不接 5V |
| GND | 任意 GND | 共地 |
| SDA | Pin3, `I2C0_SDA` | 数据线，对应 `/dev/i2c-0` |
| SCL | Pin5, `I2C0_SCL` | 时钟线，对应 `/dev/i2c-0` |
| CS/CSB | 3.3V | 拉高进入 I2C 模式，不要悬空 |
| SDO | GND | I2C 地址为 `0x68`；接 3.3V 时才是 `0x69` |
| INT1/INT2 | 先不接 | 初版轮询读取，不需要中断 |

板端先验证传感器：

```bash
cd /path/to/work/linux_bmi270_backpack
python3 bmi270_backpack.py --probe-i2c --i2c-bus 0
python3 bmi270_backpack.py --config config.ss928_ble.json --no-ble
```

确认姿态数据正常后，再启动 BLE。当前这块板实测系统默认 BlueZ 已经有 `hci0`，脚本默认直接用系统 BlueZ 注册 Nordic UART Service，不再重复执行 `/opt/sample/ws73/ble.sh 0`：

```bash
sh ./start_ss928_ble.sh
```

如果要显式确认跳过 `ble.sh`：

```bash
START_BLE_STACK=0 sh ./start_ss928_ble.sh
```

如果换了系统、默认 BlueZ 没有控制器，再用 `START_BLE_STACK=1 sh ./start_ss928_ble.sh` 走 ws73 例程。若 `ble.sh` 没有把两行 `export DBUS_*_BUS_ADDRESS=...` 打出来，就先手动复制例程中绿色的两行 export 到当前终端，再运行。

如果实测把 SDO 接到了 3.3V，或者 BMI270 在别的 I2C 总线上，只改 `config.ss928_ble.json` 里的 `i2c_addr` / `i2c_bus`，并用同样参数覆盖脚本：

```bash
I2C_BUS=0 I2C_ADDR=0x69 sh ./start_ss928_ble.sh
```

## 先确认 BMI270 是否被 Linux 识别

```bash
python3 bmi270_backpack.py --list-iio
```

你应该看到类似：

```text
/sys/bus/iio/devices/iio:device0 name=bmi270 channels=in_accel_x_raw,...
```

如果没有：

- 先确认设备树里启用了 BMI270。
- I2C 接法一般是 `SCL/SDA/GND/VCC`，SDO 决定地址 `0x68/0x69`。
- SPI 接法一般是 `SCL=SCK`、`SDA=MOSI`、`SDO=MISO`、`CS=片选`。
- 先用 `i2cdetect -y <bus>` 或内核日志 `dmesg | grep -i bmi` 查设备。

## PC 或板端无传感器测试

```bash
python3 bmi270_backpack.py --simulate --no-ble
```

会输出 JSON 数据，模拟到一定时间后会触发姿态异常。

## 板端运行

```bash
cp config.example.json bmi270_backpack.json
python3 bmi270_backpack.py --config bmi270_backpack.json
```

如果自动识别错了 IIO 设备，在配置里指定：

```json
{
  "device": {
    "iio_path": "/sys/bus/iio/devices/iio:device0",
    "sample_hz": 50.0
  }
}
```

## 输出数据格式

普通数据是一行紧凑 JSON，例如：

```json
{"t":1780926600.1,"r":1.0,"p":0.7,"y":0.6,"s":0.0,"ag":1.01,"gyr":6.9,"st":0,"a":[0.0,0.02,1.01],"w":[4.7,4.0,3.0]}
```

字段含义：

- `r/p/y`：roll、pitch、yaw，单位度。
- `s`：速度估算，单位 m/s。
- `ag`：加速度模长，单位 g。
- `gyr`：角速度模长，单位 deg/s。
- `st`：是否静止，1 为静止。
- `a`：三轴加速度，单位 g。
- `w`：三轴角速度，单位 deg/s。
- `al`：本帧触发的报警代码，可选字段。

报警会额外输出：

```text
ALERT,TILT_FORWARD,level=2,pitch=48.0,roll=1.2,speed=0.20,accel_g=1.01
```

## 当前默认阈值

这些值只是先编的，方便你后面实测调：

- 前倾 `pitch > 35 deg` 持续 `0.8 s`：`TILT_FORWARD`
- 后仰 `pitch < -35 deg` 持续 `0.8 s`：`TILT_BACKWARD`
- 左滚 `roll < -45 deg` 持续 `0.8 s`：`ROLL_LEFT`
- 右滚 `roll > 45 deg` 持续 `0.8 s`：`ROLL_RIGHT`
- 撞击 `accel > 2.8 g`：`IMPACT`
- 近似失重 `accel < 0.35 g` 持续 `0.25 s`：`FREEFALL`
- 粗略速度 `speed > 2.5 m/s` 持续 `1.0 s`：`SPEED_WARN`
- 同一报警冷却时间：`3.0 s`

修改 `config.example.json` 里的 `thresholds` 即可。

## 报警输出控制喇叭/外设

主程序不直接写喇叭驱动，只提供两个通用钩子：

### 1. 终端特殊信号

监听 stdout 里的：

```text
ALERT,<CODE>,...
```

你后续的喇叭程序、脚本、MCU 或其他进程看到这行就可以响。

### 2. 写一个文件或 GPIO value

配置：

```json
{
  "output": {
    "alert_file": "/tmp/bmi270_alert",
    "alert_active_value": "1",
    "alert_inactive_value": "0",
    "alert_pulse_ms": 300
  }
}
```

如果你的板子还保留旧 sysfs GPIO，也可以写成：

```json
{
  "output": {
    "alert_file": "/sys/class/gpio/gpio23/value"
  }
}
```

新内核更推荐 libgpiod，后续可以把 `alert_command` 配成调用 `gpioset` 的脚本。

## 手机蓝牙接收

程序默认开启 BLE，广播名：

```text
BMI270-Backpack
```

BLE 服务使用 Nordic UART Service：

- Service UUID：`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX/write UUID：`6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX/notify UUID：`6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

手机端测试方法：

1. 打开 nRF Connect 或支持 BLE UART/Nordic UART 的串口蓝牙 App。
2. 或者使用 `work/ssminiprogram` 里的微信小程序首页。
3. 扫描并连接 `BMI270-Backpack`。
4. 对 TX characteristic 开启 notify。
5. 就能收到每行 JSON 数据。

小程序和 BLE App 使用同一套 UUID：

- 小程序连接后订阅 TX/notify，实时展示 `r/p/y`。
- 小程序向 RX/write 写 `ZERO` 可以清零速度和 yaw，写 `STATUS` 可以查看阈值配置。

如果使用通用 BLE App：

1. 扫描并连接 `BMI270-Backpack`。
2. 对 TX characteristic 开启 notify。
3. 就能收到每行 JSON 数据。

手机还可以往 RX 写命令：

```text
STATUS
ZERO
ZERO_V
SET pitch_forward_deg=40
SET roll_right_deg=50
```

## 建议安装到系统

```bash
sudo mkdir -p /opt/bmi270_backpack
sudo cp bmi270_backpack.py /opt/bmi270_backpack/
sudo cp config.example.json /etc/bmi270_backpack.json
sudo cp bmi270-backpack.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now bmi270-backpack
journalctl -u bmi270-backpack -f
```

## 调试建议

1. 先 `--simulate --no-ble` 确认程序逻辑能跑。
2. 再 `--list-iio` 确认 BMI270 被 Linux IIO 识别。
3. 再不接喇叭运行，观察 JSON 的 `r/p/y/ag/s` 是否符合实际动作。
4. 根据背包实际佩戴方向，可能需要调整传感器安装方向或在程序里加轴映射。
5. 最后再接报警输出和 BLE 手机端。

## 没有 BMI270 内核驱动时的 I2C 用户态模式

如果 4.19 内核没有 `bmi270_i2c` / `bmi270_spi` / IIO 驱动，可以直接用本程序的用户态 I2C 后端。

先确认 I2C 设备：

```bash
ls /dev/i2c-*
python3 bmi270_backpack.py --probe-i2c
python3 bmi270_backpack.py --probe-i2c --i2c-bus 0
```

注意：其他板卡示例里可能写 `--i2c-bus 1`，但 SS928 旧接线的 Pin3/Pin5 是 I2C0，本项目第一版应使用 `--i2c-bus 0`。

正常 BMI270 的 chip id 是 `0x24`，地址通常是：

- `0x68`：SDO 接 GND。
- `0x69`：SDO 接 VCC。

I2C 模式接线注意：

- `SCL -> 板子 I2C SCL`
- `SDA -> 板子 I2C SDA`
- `GND -> GND`
- `VCC -> 3.3V`
- `CS -> 3.3V`，I2C 模式不要悬空
- `SDO -> GND` 或 `3.3V`，决定地址 `0x68/0x69`

如果 probe 能看到 `chip_id=0x24`，直接运行：

```bash
python3 bmi270_backpack.py --backend i2c --i2c-bus 0 --i2c-addr 0x68 --no-ble
```

或者修改配置：

```json
{
  "device": {
    "backend": "i2c",
    "i2c_bus": 0,
    "i2c_addr": "0x68",
    "config_blob": "auto",
    "init_sensor": true,
    "sample_hz": 50.0
  }
}
```

用户态 I2C 模式需要 `bmi270_config.bin` 和 `bmi270_backpack.py` 放在同一个目录下。这个二进制配置文件已经从 STM32 示例里的 `BMI270_config.h` 生成，用来初始化 BMI270 内部特性固件。
