### work/linux_bmi270_backpack + work/ssminiprogram

- 目标：先打通 SS928 读取 BMI270 姿态数据并通过 BLE 发到微信小程序实时展示的最小链路。
- 板端方案：复用 `work/linux_bmi270_backpack/bmi270_backpack.py`，它输出紧凑 JSON，并作为 Nordic UART Service BLE 外设广播 `BMI270-Backpack`；新增 `config.ss928_ble.json` 和 `start_ss928_ble.sh`。
- 接线基准：沿用 `work/bmi270_i2c_pose` 的旧接线，SDA=Pin3/I2C0_SDA，SCL=Pin5/I2C0_SCL，CS/CSB 拉 3.3V，SDO 接 GND，地址 `0x68`，对应 `/dev/i2c-0`。不要把通用 Python 示例里的 I2C1 当成这块板当前接线。
- BLE 启动：`start_ss928_ble.sh` 会先设置 I2C0 pinmux。当前板子实测系统 BlueZ 可用，脚本默认跳过 `ble.sh` 直接注册 GATT；只有没有系统控制器时才用 `START_BLE_STACK=1` 切到 `/opt/sample/ws73` 执行 `ble.sh 0` 并解析 DBus export 行。
- 小程序方案：`work/ssminiprogram/miniprogram/pages/index` 已从云开发 quickstart 首页改成原生微信 BLE 面板，流程为打开蓝牙、按 NUS UUID 扫描、连接、发现服务/特征、订阅 TX notify、按换行重组 BLE 分片并解析 JSON，展示 roll/pitch/yaw、六轴原始值、状态、告警、帧率，并可向 RX 写 `ZERO` / `STATUS`。
- 验证结果：Windows 本机已通过 `node --check` 检查小程序 JS，通过 `python -m json.tool` 检查相关 JSON，用 Git `sh -n` 检查启动脚本语法，并用 `--simulate --no-ble` 确认板端仍输出含 `r/p/y` 的 JSON。实际 I2C、BlueZ GATT 注册和微信真机 BLE 连接必须在 SS928 和手机上验证。
- 踩坑：微信开发者工具模拟器不能替代真机 BLE；如果小程序扫描不到，先用 nRF Connect 或 bluetoothctl 看板端是否真的广播 `BMI270-Backpack` 和 NUS UUID，再查 `ble.sh` 的 DBus 环境是否被 Python 进程继承。
