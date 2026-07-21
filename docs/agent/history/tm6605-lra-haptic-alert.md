# TM6605 双 LRA 触觉预警（2026-07-19）

## 目标

将 SmartBag 原有四路 PWM 震动模块替换为左、右各一块 TM6605 LRA 触觉驱动模块，并保持原有四级预警入口。

## 硬件事实与接线

- TM6605 是 I2C 控制、固定 7-bit 地址 `0x2D` 的 LRA 驱动器，不接受 GPIO PWM 作为震动强度输入。
- 两块 TM6605 地址相同，必须通过 TCA9548A I2C 多路复用器隔离；40Pin I2C0 的 Pin3/SDA、Pin5/SCL 接 TCA9548A，通道 0 给左侧、通道 1 给右侧。
- 两块 TM6605 与 TCA9548A 使用 5V；SS928 Pin3/5 至 TCA9548A 主线间加入双向 I2C 电平转换器（3.3V/5V），所有 GND 相连。不得让模块的 5V I2C 上拉直接接入 SS928。
- 旧震动 PWM 引脚 Pin7、Pin32、Pin35、Pin37 不再使用。

## 最终行为

- 1 级播放一次轻撞击 effect 7（约 130 ms），不播放语音，不亮灯。
- 2 级播放两次轻撞击 effect 7，两次启动间隔 250 ms，不播放语音，不亮灯。
- 3 级连续播放两段 effect 15（各约 730 ms、第二段在 750 ms 启动），总时长约 1.48 秒；播放对应三级语音，灯亮 1 秒。
- 4 级以 250 ms 启动间隔播放三次剧烈警报 effect 14；播放对应四级语音，灯闪三次。
- TM6605 自动在 LRA 共振频率工作，强弱和波形由内置 effect 选择，不能映射为 GPIO PWM 占空比。

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

## 2026-07-20 实机复测

- 已通过 SSH 连接实物 SS928；I2C0 的 Pin3/Pin5 复用分别读回 `0x2031`，并实际向 `0x2D` 下发了左、右各自 L3（effect 15）与 L4（effect 14）控制帧。
- 未经 TCA9548A 直接写 `0x2D` 时，左右控制均返回 `OSError: [Errno 5] Input/output error`；这证明当前实物不应使用直连模式。
- 复测确认 TCA9548A 位于 `0x70`：通道 1 的左 TM6605、通道 2 的右 TM6605 均 ACK 三级 effect 15 和四级 effect 14 三脉冲。两块同址设备必须持续通过该多路器独立控制。
- 启动脚本现已把 `SMARTBAG_TM6605_USE_MUX=1` 与 `SMARTBAG_ENABLE_RIGHT_TM6605=1` 传给控制器；`smartbag-alert.service` 已重新启动并保持 active。

## 2026-07-21 四级输出映射调整

- 本地控制逻辑已改为上述新映射，并增加一级/二级轻震、三级 1.5 秒停止、四级快速三脉冲和一级/二级无语音无灯光的回归测试。
- 本轮只完成本地代码与测试，尚未上传、重启服务或对新 effect 7/118 时序做实物体感复测；2026-07-20 的板端结果只证明旧 effect 15/14 映射和左右硬件链路当时可用。

## 2026-07-22 板端融合部署与受控触觉验证

- 先完整拉取板端 `/root/work/smartbag_alert_controller`；与本地融合后，主控制器、BLE、PWM、雷达和启动脚本没有冲突，上传后的关键源文件 SHA-256 与本地一致。
- 板端 `sh -n start_ss928_smartbag_alert.sh` 和 30 项 Python 单元测试通过；`smartbag-alert.service` 重启后为 `enabled`、`active/running`、`NRestarts=0`，继续监听 `192.168.1.102:2368`。
- 通过 TCA9548A `0x70` 的左通道 1、右通道 2 实际下发了左 L1/右 L2，以及左 L3 1.5 秒/右 L4 三脉冲；脚本均返回完成标记并在结束时显式停止所有播放，服务保持运行。TM6605 是只写驱动，命令成功不等同于人为体感、灯光可见或语音可闻确认。

## 2026-07-22 三级时长修正

- 现场实测表明 effect 118 实际持续十余秒；原因是它是芯片内置长警报，播放控制寄存器的停止写入不能可靠中断已经启动的该效果。
- 三级改为两段 effect 15（约 730 ms），第二段在 750 ms 启动，总时长约 1.48 秒；不再使用 effect 118，也不依赖中途停止写入。
- 已只上传 `tm6605_haptics.py`、对应回归测试和控制器 README；发现并同步了与板端音频源码不配套的旧测试后，板端完整 30 项测试通过。
- `smartbag-alert.service` 重启后为 `active/running`、`NRestarts=0` 并保持 UDP 监听。已对左侧三级执行一次受控实物序列，两个 effect 15 在 0 与 0.75 秒下发，脚本调度总耗时 1.62 秒（含 0.14 秒结束余量）；仍需现场人员确认主观震感时长。
