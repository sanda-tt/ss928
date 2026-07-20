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
