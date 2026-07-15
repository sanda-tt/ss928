### work/dx_gp21_tracker + work/ssminiprogram tracks

- 目标：把补充资料里的 DX-GP21-A GNSS 模块接到 SS928 40Pin UART4，板端读取 NMEA、保存 WGS84 轨迹，并通过微信小程序 BLE 近场查看实时位置和历史轨迹。
- 推荐接线：DX-GP21-A 底板 `VDD` 接 40Pin Pin2/Pin4 的 5V，`GND` 接任意 GND，`TXD` 接 Pin10 `UART4_RXD`，`RXD` 接 Pin8 `UART4_TXD`；首版不接 `1PPS`、`ON/OFF`、`VBAT`。若是裸模块 `VCC` 只能接 3.3V，不能接 5V。
- 关键文件：`work/dx_gp21_tracker/dx_gp21_tracker.py` 是 stdlib Python 板端服务；`config.ss928_uart4.json` 固定 `/dev/ttyAMA4`、115200、`DX-GP21-Track`；`start_ss928_gnss_track.sh` 设置 `bspmm 0x102F0134 0x1201` 和 `0x102F0138 0x1201`；`README.md` 写明接线、NMEA 验证和 USB-TTL fallback。
- 小程序方案：复用 `work/ssminiprogram/miniprogram/pages/tracks` 的 Nordic UART Service 页面，扫描名加入 `DX-GP21-Track`，命令协议为 `TL` 轨迹列表、`TG <i> <offset>` 分页轨迹、`TF 1/0` 实时位置、`TS` 状态。轨迹点仍按现有工具从 WGS84 转 GCJ-02 后绘制到微信地图。
- 40Pin 占用：`40pin_usage.md` 已记录 Pin8/Pin10 为 `UART4` 给 DX-GP21-A 使用；后续不要再把 60GHz 雷达或其他 UART4 外设同时接到这组脚，除非明确切换硬件方案。
- 验证结果：本地 `python -B -m unittest discover -s work/dx_gp21_tracker/tests` 通过 5 个测试；小程序 `track-utils.test.js` 通过；`node --check pages/tracks/index.js` 和 `sh -n start_ss928_gnss_track.sh` 通过。板端 SSH 只读探测确认 `/dev/ttyAMA4`、`bspmm`、Python 3.10、BlueZ controller 和 `dbus/gi` 可用；上传到 `/root/dx_gp21_tracker` 后，板端单测、脚本语法和 `--simulate --once --no-ble` 协议帧也通过。
- 未验证项：尚未接实物 DX-GP21-A 做真实 NMEA 和室外 fix 验证。接线后先运行 `timeout 10 python3 dx_gp21_tracker.py --config config.ss928_uart4.json --no-ble --dump-nmea`，看到 `$GNGGA`/`$GNRMC` 后再启动 BLE；陶瓷/无源天线输出 `ANTENNA OPEN` 不必直接判故障。
