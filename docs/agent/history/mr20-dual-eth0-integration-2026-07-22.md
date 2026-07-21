# MR20 双侧 eth0/eth1 接入（2026-07-22）

## 目标

保留右侧 MR20 的 eth1 链路，同时让左侧 MR20 可直接接入 eth0；eth0 断开雷达并接入电脑时仍可通过 SSH 调试。

## 最终配置

- 右侧保持不变：`192.168.1.200:2369 -> 192.168.1.102:2368`（eth1）。
- 左侧：`192.168.1.201:2379 -> 192.168.1.168:2378`（eth0）。
- eth0 保持板端静态地址 `192.168.1.168/24`，未修改板端网络或默认路由。
- `mr20_radar.json` 中启用两个独立 worker；它们分别按绑定地址、端口和来源 IP 过滤数据，并写入左右独立 JSONL 日志。

## 关键文件

- `work/smartbag_alert_controller/mr20_radar.example.json`
- `work/smartbag_alert_controller/tests/test_mr20_radar.py`
- 板端：`/root/smartbag_alert/config/mr20_radar.json`

## 验证结果

- 本地 MR20 测试 7 项、SmartBag Alert 全量测试 31 项通过。
- 板端配置已备份为 `/root/smartbag_alert/config/mr20_radar.json.bak-20260722-dual` 并完成替换。
- `smartbag-alert.service` 为 `enabled`、`active`；同一 Python 进程监听 `192.168.1.102:2368` 和 `192.168.1.168:2378`。
- 服务重启后仍可通过 eth0 SSH 继续执行板端验证；`smartbag-5g-upload.service` 保持 `active`，未启动旧 BMI/BlueZ 入口。

## 未验证项

- 部署时 eth0 仍承载当前 SSH 调试连接，尚未接入左侧实物雷达，因此没有左侧 `192.168.1.201:2379` 的真实 UDP 帧或目标告警证据。接入后检查 `mr20_left_rear.jsonl` 与服务日志中的 `left_rear` 记录。

## 语音仲裁修复（2026-07-22）

- 高等级新告警不再终止正在播放的低等级语音，避免语音播半句；它会作为唯一待播语音，在当前句完整结束后播放。
- `AL CLEAR` 和服务停止仍会立即终止当前播放。音频定向测试 3 项及 SmartBag Alert 全量测试 31 项通过；部署后应确认服务日志和实际语音播放。
