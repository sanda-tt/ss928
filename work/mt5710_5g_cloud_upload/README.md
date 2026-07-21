# MT5710 5G SmartBag CloudBase 上传启动器

## 用途

SS928 接好 MT5710、DX-GP21-A 和 BMI270 后，运行一个命令即可：

1. 检查 MT5710 SIM、驻网和数据附着；
2. 自动选择移动 `cmnet`、电信 `ctnet` 或联通 `3gnet`，也可用 `--apn` 覆盖；
3. 拨号、动态发现 `cdc_ncm` 网口并运行 DHCP；
4. 确认 CloudBase 目标流量实际走 5G，而不是板载以太网；
5. 启动现有 DX-GP21-A 和 BMI270 真实采集程序。

上传内容仍由现有代码决定：DX-GP21-A 有效轨迹、BMI270 处理后的
`good/bad` 姿态和每日统计，以及既有规则最终确认的
`FALL_ALARM/fall_detected`。MT5710 不参与定位。

## 板端准备

需要以下目录位于 `/root/work`：

```text
smartbag_cloud_uploader/
dx_gp21_tracker/
linux_bmi270_backpack/
imu_fall_detector/
mt5710_5g_cloud_upload/
```

把上传令牌和跌倒告警电话号码放入不进入 Git 的 root-only 文件：

```sh
# BOARD-LINUX
mkdir -p /root/config
chmod 700 /root/config
printf '%s\n' 'SMARTBAG_UPLOAD_TOKEN=<比赛上传令牌>' > /root/config/smartbag-upload.env
printf '%s\n' 'SMARTBAG_ALERT_PHONE=<告警电话号码>' >> /root/config/smartbag-upload.env
chmod 600 /root/config/smartbag-upload.env
```

只有最终 `FALL_ALARM/fall_detected` 会额外通过 MT5710 发送一条 UCS2/PDU
中文短信；普通预警不会发短信，也不会拨号。短信异常只记警告，不影响本地告警
和原有 CloudBase 上传。未得到再次发送真实短信的授权时，只运行 fake AT 单元
测试，不要创建手动跌倒触发文件。

## 一个命令运行

```sh
# BOARD-LINUX，root 用户
cd /root/work/mt5710_5g_cloud_upload
./start_ss928_5g_upload.sh
```

## 开机自启动

只需安装一次：

```sh
# BOARD-LINUX，root 用户
cd /root/work/mt5710_5g_cloud_upload
./install_autostart.sh
```

安装器会启用 `smartbag-5g-upload.service`，并禁用旧的
`bmi270-backpack.service`，避免两个进程同时读取 BMI270。BMI270 仍通过现有
`start_ss928_ble.sh` 启动，因此保留 BLE 和 CloudBase 上传。

> 单一入口约束：不要再手工启动 `bmi270-backpack.service`，也不要在本
> 服务运行时另开 `start_ss928_ble.sh`、`bmi270_backpack.py --ble` 或
> `/opt/sample/ws73/ble.sh 0`。这些旧/独立入口会产生第二个 BMI270 进程
> 或第二个 `bluetoothd`。旧 unit 文件可以保留，但必须保持
> `disabled`、`inactive`；系统 BlueZ 由 `bluetooth.service` 唯一管理。

该服务是后台 `Type=simple` 服务，不等待 `network-online.target`，不会阻塞
系统启动。MT5710 或传感器尚未就绪时，本次执行按有界超时退出，5 秒后自动重试。

> 当前 DX-GP21-A 已拔除并被标记为故障隔离：随附的
> `smartbag-5g-upload.service` 固定使用 `--skip-gnss`，不会检查
> `/dev/ttyAMA4`、设置 UART4 GNSS pinmux 或拉起 GNSS 进程；5G、BMI270、摔倒
> 告警及其上传继续运行。模块修复并重新接线后，先单独完成 NMEA 验证，再移除 unit
> 中的 `--skip-gnss` 并重启服务；不要在当前状态下反复调试 DX-GP21-A。

```sh
# BOARD-LINUX：查看状态和日志
systemctl status smartbag-5g-upload.service --no-pager -l
journalctl -u smartbag-5g-upload.service -b --no-pager -n 100
systemctl is-enabled bmi270-backpack.service  # 期望 disabled
systemctl is-active bmi270-backpack.service   # 期望 inactive
pgrep -a -f 'bmi270_backpack.py|bluetoothd'   # 各自只应有一个

# 如需取消自启动
systemctl disable --now smartbag-5g-upload.service
```

仅验证 MT5710、DHCP 和 CloudBase 5G 路由，不启动传感器：

```sh
# BOARD-LINUX
./start_ss928_5g_upload.sh --check-only
```

无法自动判断专网卡 APN 时使用：

```sh
# BOARD-LINUX
./start_ss928_5g_upload.sh --apn <运营商提供的APN>
```

正常成功标记类似：

```text
5G ready: interface=enx... apn_configured=yes cloud_ip=...
Starting real DX-GP21-A and BMI270 CloudBase producers
Started pid=...: dx_gp21_tracker.py
Started pid=...: bmi270_backpack.py
```

按 `Ctrl+C` 时只终止本启动器拉起的两个采集进程。

## 已验证结果（2026-07-21）

- 当前实物 MT5710 识别为中国电信，自动选择 `ctnet`。
- NCM 网口动态识别为 `enx2a5a7626cdfa`，DHCP 地址为 `10.118.237.237`。
- CloudBase IPv4 `124.223.146.214` 的路由为 `dev enx2a5a7626cdfa`。
- 通过该 5G 路由发送轨迹、姿态/每日统计、摔倒三类测试 telemetry，均返回 HTTP 200。
- CloudBase 四个集合及 `smartbag-app-api` 四个读取 action 均回读成功。
- 已有数据会话时可重复执行，模块返回 `ERROR: DUPLICATED` 会被识别为已连接并继续验证路由。
- 本次只有 MT5710 已连接；DX-GP21-A、BMI270 实物采集仍需接线后运行上述一个命令验证。

`udhcpc` 可能临时改变板端默认路由和 DNS；拨号程序不写永久网络配置，只有明确运行
`install_autostart.sh` 时才安装上述 systemd 自启动项。

## 严重摔倒短信更新（2026-07-22）

- 已撤销语音拨号和音频播放方案；最终严重摔倒改为向 root-only 环境变量中的
  号码发送一条中文短信。
- 中文短信使用 `AT+CMGF=0` 的 UCS2/PDU 模式；只有同时返回 `+CMGS:` 和 `OK`
  才算提交成功。超时或结果不确定时不自动重试，避免重复报警。
- LOCAL-WIN 的 MT5710、BMI270、跌倒融合测试分别为 20/20/11 项通过；
  BOARD-LINUX 的 MT5710、BMI270 测试分别为 20/20 项通过。
- 7 个部署文件的 SHA-256 全部一致，`smartbag-5g-upload.service` 已加载新代码并
  稳定运行，5G、BMI270、BLE GATT 和广播日志正常；旧 BMI unit 仍为
  `disabled`、`inactive`。
- 用户已确认此前同一短信链路实际收到过中文报警短信。本次部署复核没有再次制造
  严重摔倒事件或发送短信，避免重复提醒。
- 号码缺失或格式非法时只禁用短信 notifier，并输出不含号码/PDU 的警告；
  BMI270、本地告警和 CloudBase 上传继续运行。
- 2026-07-22 实机触发发现冷 PCUI 直接从 `AT+CMGF=0` 开始可能立即失败；正式
  路径现先发送 `AT` 做唤醒/健康检查。修复后脱敏实发同时获得 `+CMGS:` 与 `OK`，
  且收件端确认实际收到报警短信。
