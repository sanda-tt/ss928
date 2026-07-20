# MT5710 SmartBag 自启动设计

## 目标

SS928 在 MT5710、DX-GP21-A、BMI270 均接好后上电，无需人工命令即可通过
5G 上传轨迹、处理后姿态、每日姿态统计和最终摔倒告警；任何外设缺失或异常都不能阻塞系统启动。

## 方案

新增单一 `smartbag-5g-upload.service`，由 `multi-user.target` 拉起现有
`/root/work/mt5710_5g_cloud_upload/start_ss928_5g_upload.sh`。服务使用
`Type=simple`，不成为其他系统服务的依赖，也不等待 `network-online.target`，因此系统启动目标不会等待拨号或传感器初始化。

服务使用 `Restart=always` 和 `RestartSec=5`。MT5710 尚未枚举、SIM 未驻网、DHCP
失败或传感器进程退出时，本次运行按现有有界超时结束，由 systemd 稍后重新尝试。
`TimeoutStopSec=10` 限制停止等待时间，`KillMode=control-group` 确保两个采集子进程随服务一起停止。

## 进程和兼容性

统一服务负责 MT5710 建链及 DX、BMI 两个生产者，避免拆成多套启动依赖。BMI
生产者改为复用现有 `start_ss928_ble.sh`，保留既有 BLE 行为和 CloudBase 上传。
部署时停止并禁用旧 `bmi270-backpack.service`，防止两个进程同时占用 BMI270；
不修改 `smartbag-alert.service` 和其他启动项。

## 安全边界

- 上传令牌继续只从 `/root/config/smartbag-upload.env` 读取，不写入 unit 或仓库。
- 不写永久默认路由；现有启动器只验证/临时修正 CloudBase 目标路由。
- 不使用 `ExecStartPre` 无限轮询，不使用阻塞式 `oneshot` 服务。
- 日志进入 systemd journal，便于用 `journalctl -u smartbag-5g-upload.service` 查看。

## 验证

先用自动测试检查 unit 的非阻塞、重试、超时、凭据路径和启动命令，再部署到板端。
板端先启动并检查服务状态与日志；最后经用户允许执行一次重启验证，确认系统可正常启动、服务在后台运行且外设缺失时只重试不阻塞。
