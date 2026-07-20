# MT5710 SmartBag 真实数据上传设计

## 目标

提供一个比赛 MVP 启动入口：SS928 接好 MT5710、DX-GP21-A 和 BMI270 后，执行一个命令即可建立 5G NCM 网络，并把现有采集程序产生的全部业务数据上传到现有 CloudBase telemetry 接口。

## 范围

- 上传 DX-GP21-A 有效 GNSS 轨迹。
- 上传 BMI270 现有规则处理后的 `good/bad` 姿态和每日绝对统计。
- 仅上传现有摔倒融合逻辑最终输出的 `FALL_ALARM/fall_detected`。
- MT5710 只负责网络传输，不提供定位。
- 不生成模拟数据，不修改 CloudBase 协议，不新增认证或云资源。
- 本轮没有连接外设，交付以自动化测试和静态检查为准；真实硬件结果必须等接线后验证。

## 方案比较

1. 推荐：新增一个标准库 Python 启动器。它负责 AT 检查、APN 选择、NCM 拨号、DHCP、路由验证和两个现有采集进程的监督。代码可测试、错误信息明确，仍保持一个命令启动。
2. 纯 Shell 脚本。代码更短，但串口 AT 读写、超时和响应解析不可靠，也很难在无硬件时测试。
3. 把 5G 逻辑写进两个采集程序。会重复代码并耦合传感器与网络，不符合 MVP 的最小改动原则。

采用方案 1。

## 组件和数据流

新增 `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`：

1. 检查 root、`SMARTBAG_UPLOAD_TOKEN`、PCUI 串口和两个传感器程序。
2. 从 `AT+COPS?`/`AT+CIMI` 判断运营商；允许 `--apn` 覆盖，默认映射移动 `cmnet`、电信 `ctnet`、联通 `3gnet`。
3. 确认 SIM ready、已驻网和 `CGATT=1`，发送 `AT^NDISDUP=1,1,"<apn>"`。
4. 从 sysfs 动态发现 `cdc_ncm` 网口，执行 `ip link set` 和 `udhcpc`。
5. 解析 CloudBase 主机 IPv4，并要求 `ip route get` 的出口为该 NCM 网口；不满足时退出，避免把以太网上传误报成 5G 上传。
6. 设置 UART4/I2C0 pinmux，使用 `--no-ble` 启动现有 DX-GP21-A 和 BMI270 程序。两个程序继续复用 `smartbag_cloud_uploader`，上传失败不终止采集。
7. 捕获退出信号并只终止自己启动的两个子进程。

提供 `start_ss928_5g_upload.sh` 作为一个命令入口。它可从 Git 外的 `/root/config/smartbag-upload.env` 读取令牌，再执行 Python 启动器。

## 错误处理

- 缺串口、SIM 未 ready、未驻网、未 attach、拨号失败、NCM/DHCP/路由失败或缺传感器文件时立即退出并给出单行原因。
- 不打印令牌、完整请求体或串口无关数据。
- 不编辑 systemd、启动项、永久网络配置或 `/etc` 文件。
- `udhcpc` 可能在本次运行中改变默认路由和 DNS；这是临时运行影响，重启后恢复。

## 验证

无硬件阶段：单元测试覆盖 APN 映射、AT 状态解析、NCM 动态发现、路由判定、真实采集命令构造和缺失条件失败；运行现有上传器、DX-GP21-A、BMI270 回归测试及语法检查。

接线后：运行一个命令，要求 AT 驻网、DHCP、CloudBase 路由检查通过；等待真实 GNSS/姿态数据；再从 CloudBase 回读 `track_points`、`device_status`、`posture_daily_stats`，并只在真实最终摔倒触发后验证 `alarm_history`。
