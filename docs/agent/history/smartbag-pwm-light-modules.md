# SmartBag 双侧 PWM 光模块（2026-07-20）

## 目标

在既有左右四级预警控制器上增加两个小信号 PWM 光模块输出。

## 引脚与行为

- 左侧光模块：40Pin Pin7，`PWM0_OUT10_0_P`（PWM chip 0 / channel 10），pinmux `bspmm 0x102F0110 0x1205`。
- 右侧光模块：40Pin Pin32，`PWM0_OUT1_0_P`（PWM chip 0 / channel 1），pinmux `bspmm 0x102F01EC 0x1201`。
- 三级：对应侧以 50% 占空比输出 1 秒。
- 四级：对应侧以 80% 占空比输出三次脉冲；节奏为亮 200 ms、灭 200 ms。
- 1/2 级、`AL CLEAR` 和事件超时均立即关闭对应灯。

Pin7/Pin32 已在 TM6605 迁移后从旧震动 PWM 输出释放；Pin3/5 I2C、Pin8/10 UART4、Pin12/38/40 I2S 保持不变。

## 实现与验证

- 新增 `work/smartbag_alert_controller/pwm_lights.py`，使用板端 sysfs PWM，并在控制器启动时配置两路 pinmux、导出 PWM 通道和设置 1 kHz 周期。
- `smartbag_alert_controller.py` 通过已有 `AlertOutput.levels` 同时驱动触觉、音频和光模块，不改变预警融合规则。
- 本机和板端均通过 `python3 -m py_compile smartbag_alert_controller.py pwm_lights.py` 与 20 项单元测试。
- 板端只读确认存在 `pwmchip0`（16 通道）与 `pwmchip16`（16 通道）；更新已同步至 `/root/smartbag_alert/controller`。

## 未验证项

- 未接入两块实物光模块，尚未执行非 dry-run 的 pinmux/PWM 输出；接线完成后用 `AL L3`、`AL R4` 逐侧验证，确认模块输入是 3.3V 小信号且与 SS928 共地。

## 2026-07-20 实机复测

- 实物板 `pwmchip0` 的 channel 10（Pin7）与 channel 1（Pin32）均已成功导出和配置为 1 kHz。
- 两侧均已实际执行 L3 的 50% PWM 持续 1 秒，以及 L4 的 80% PWM 三次 200 ms 脉冲；每次后均读回 `enable=0`，输出已安全关断。
- 原服务的首次失败发生在右侧 PWM 刚导出后的初始化阶段；现已在导出后轮询 sysfs 节点，两个通道均可正常配置。配合 TCA9548A 后，完整控制服务已保持运行。
