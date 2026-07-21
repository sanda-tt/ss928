# MT5710 5G SmartBag CloudBase 上传

## 目标

提供一个比赛 MVP 启动入口，让 SS928 在外设接好后通过 MT5710 NCM 5G 网络上传
DX-GP21-A 轨迹、BMI270 处理后姿态/每日统计和最终摔倒告警。

## 最终方案

- `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py` 使用标准库完成 AT、APN、拨号、NCM、DHCP、路由验证和采集进程监督。
- `start_ss928_5g_upload.sh` 从 `/root/config/smartbag-upload.env` 加载 Git 外令牌并提供一个命令入口。
- `smartbag-5g-upload.service` 以非阻塞 `Type=simple` 方式开机启动，失败后每 5 秒重试。
- `install_autostart.sh` 安装该 unit，并禁用旧 BMI270 unit 以避免重复占用传感器。
- 运营商/APN 自动映射：移动 `cmnet`、电信 `ctnet`、联通 `3gnet`；专网卡可用 `--apn` 覆盖。
- NCM 网口按 `cdc_ncm` 驱动动态发现，不固定写死 `usb0` 或某个 `enx...` 名称。
- 如果以太网仍抢占 CloudBase 路由，只临时增加 CloudBase IPv4 `/32` NCM 路由并在退出时清理；不替换全局默认路由，不写永久网络配置。
- 真实业务数据仍由现有 DX-GP21-A/BMI270/摔倒模块产生，启动器不复制算法也不生成模拟数据。

## 关键文件

- `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`
- `work/mt5710_5g_cloud_upload/start_ss928_5g_upload.sh`
- `work/mt5710_5g_cloud_upload/tests/test_mt5710_5g_cloud_upload.py`
- `work/mt5710_5g_cloud_upload/README.md`

## 验证结果

- LOCAL-WIN：11 个 MT5710/APN/NCM/路由/命令构造测试通过。
- BOARD-LINUX：同一套 11 个测试和 Python 语法检查通过。
- 实物 MT5710 已驻电信 NR-5GC，自动使用 `ctnet`。
- 实际 NCM 为 `enx2a5a7626cdfa`，DHCP 地址 `10.118.237.237`。
- `ip route get 124.223.146.214` 明确返回 `dev enx2a5a7626cdfa`。
- 通过 5G 上传三类完整测试 telemetry，三次请求均 HTTP 200。
- CloudBase 已回读 `sim-track-1784564090674`、`bag001_2026-07-20`、`sim-fall-1784564090674`，最新状态时间戳为 `1784564090674`。
- `smartbag-app-api` 的实时姿态、每日统计、轨迹、摔倒历史四个 action 均匹配本次记录。
- 已安装并启用 `smartbag-5g-upload.service`，旧 `bmi270-backpack.service` 已禁用。
- 实际重启后 systemd 状态为 `running`，`multi-user.target` 在 7.633 秒到达，整个启动为 11.640 秒；5G 随后自动输出 `5G ready`，证明采集初始化没有阻塞系统启动。
- DX/BMI 未连接时 BMI 返回 I/O error，统一服务按 `RestartSec=5` 自动重启，符合外设缺失时后台重试的设计。

## 未验证项

- 本次只有 MT5710 已连接，未连接 DX-GP21-A 和 BMI270，因此没有声称真实传感器读数已通过 5G 上传。
- 真实最终摔倒动作仍未执行。
- 小程序服务层已回读成功；最终页面显示由用户刷新真机/开发者工具确认。

## 踩坑和禁止路线

- 不要用 `eth0` 连通性冒充 5G 上传，必须检查 CloudBase 目标 `ip route get` 的 `dev`。
- 不要固定 NCM 网口名称；本次名称与历史验证不同。
- 已存在数据会话时重复拨号会返回 `ERROR: DUPLICATED`；应复用该会话并继续验证 DHCP/路由。
- 不要用 `oneshot` 或 `network-online.target` 无限等待外设；后台服务失败重试即可。
- APN 属于 SIM/运营商；本次电信卡用 `ctnet`，不能继续照抄历史移动卡的 `cmnet`。
- 不要把 MT5710 当定位源；轨迹只来自 DX-GP21-A。
- 不要把令牌写入代码、仓库文档或命令日志。
- 不要重新启动或启用旧 `bmi270-backpack.service`；它只保留作历史/独立
  示例，当前整机必须为 `disabled`、`inactive`。
- `smartbag-5g-upload.service` 运行时不要另开 `start_ss928_ble.sh`、
  `bmi270_backpack.py --ble` 或 `/opt/sample/ws73/ble.sh 0`。统一服务已经
  以 `START_BLE_STACK=0` 拉起唯一 BMI 进程，并复用系统 BlueZ。
- 2026-07-22：DX-GP21-A 已因模块故障拔除。统一服务固定传入 `--skip-gnss`，
  因此不检查 UART4、不设置 GNSS pinmux、不启动 GNSS；5G、BMI270 和摔倒链路
  必须继续独立运行。模块修复前不要把无 GNSS 数据当作其他流程故障。

## 2026-07-22 阶段性语音电话告警（已被短信方案取代）

- 最终 `FALL_ALARM/fall_detected` 在保留本地告警和 CloudBase 上传后，独立调用
  MT5710 `ATD<号码>;` 发起语音呼叫；普通预警不拨号。
- 已删除本轮新增的短信、PDU、`AT+CMGS` 路径，不再发送报警短信。
- 电话号码仅从 root-only 服务环境变量读取，仓库配置和日志不保存真实号码。
- 本地 MT5710/BMI270/跌倒融合测试分别为 19/20/11 项通过；板端 MT5710 和
  BMI270 测试分别为 19/20 项通过，部署文件 SHA-256 与本地一致。
- `smartbag-5g-upload.service` 重启后一次因 PCUI 瞬时无响应自动重试，随后稳定
  输出 `5G ready`、BMI270 连续采样、BLE GATT 和广播成功；旧 BMI unit 保持
  `disabled`、`inactive`。
- 按用户要求本阶段只写拨号逻辑，没有制造最终摔倒事件或真实拨号。板端探测为
  `no soundcards`，电话接通后的录音播放仍需结合 MT5710 PCM/语音硬件通道验证。
- 缺失或非法电话号码会在打开 PCUI 前仅禁用电话 notifier，日志不包含号码，
  BMI270、本地告警和 CloudBase 路径继续运行。

## 2026-07-22 最终严重摔倒短信告警

- 用户最终选择短信提醒，取消语音电话和音频播放。只有最终
  `FALL_ALARM/fall_detected` 在本地告警和 CloudBase 上传后独立发送一条中文
  短信；普通预警、原始阈值事件和其他 `alarmType` 不发送，也不拨号。
- MT5710 使用 PDU 模式：`AT+CMGF=0`、等待 `AT+CMGS` 提示符、发送 DCS `08`
  的 UTF-16BE UCS2 PDU 与 Ctrl-Z；`CMGS` 长度不含 SMSC 长度字节。只有同时
  收到 `+CMGS:` 和 `OK` 才算提交成功，结果不确定时不自动重试。
- 号码仍只从 root-only 服务环境变量读取。仓库、测试、日志和本文档均不保存真实
  号码或完整 PDU；号码缺失或非法只禁用短信路径，不影响本地或云端流程。
- 用户确认此前已实际收到同一 PDU 中文报警短信，可作为该硬件短信通道的既有物理
  证据；本次切换部署没有再次制造严重摔倒事件或发送短信，避免重复提醒。
- LOCAL-WIN 的 MT5710/BMI270/跌倒融合测试分别为 20/20/11 项通过；
  BOARD-LINUX 的 MT5710/BMI270 测试分别为 20/20 项通过，7 个部署文件的
  SHA-256 与本地全部一致。
- `smartbag-5g-upload.service` 重启后保持 `active`、`NRestarts=0`，5G ready、
  BMI270 producer、BLE GATT 与广播均正常；旧 `bmi270-backpack.service` 保持
  `disabled`、`inactive`。
- 后续实机触发暴露出冷 PCUI 直接以 `AT+CMGF=0` 作为首命令时会出现立即失败。
  加入无副作用的 `AT` 唤醒/健康检查后，脱敏诊断实发同时收到 `+CMGS:` 和 `OK`，
  用户确认手机端实际收到报警短信。修复经本地与板端 MT5710 测试各 20 项通过，
  同步文件哈希一致，唯一服务重启后保持 `active`、`NRestarts=0`。

## 2026-07-22 摔倒告警默认位置与 HTTP 400 复核

- 线上 `smartbag-device-ingest` 当前校验契约允许告警不带 `location`，因此此前
  `HTTP 400` 不是缺少定位直接造成。唯一服务重启并加载当前载荷代码后，姿态探测
  请求恢复 `HTTP 200`，最近运行日志不再出现 `HTTP 400`。
- 按用户要求，`PostureCloudReporter.report_fall()` 在最新定位不存在、非法或过期时
  回退到四川大学望江校区中心点 `30.630838, 104.083932`；存在新鲜定位时仍优先
  使用真实位置。
- 本地 uploader/BMI270 测试分别 5/20 项通过，板端 BMI270 测试 20 项通过；部署
  文件 SHA-256 与本地一致。无定位的真实 HTTPS 摔倒告警返回 `HTTP 200`，随后
  CloudBase MCP 从 `alarm_history` 回读到同一 `requestId`、`fall_detected` 和上述
  默认坐标。该验证只上传云端记录，没有再次发送短信。
