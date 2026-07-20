# DX-GP21 GNSS Tracker for SS928

这个目录把 DX-GP21-A GNSS 模块接到 SS928/HiEuler Pi 的 40Pin UART4，板端读取 NMEA，保存 WGS84 轨迹，并通过 BLE Nordic UART Service 给微信小程序轨迹页查看。

## 接线

推荐接 DX-GP21-A 底板排针，不直接焊裸模块。

| DX-GP21-A 底板 | SS928 40Pin | 说明 |
| --- | --- | --- |
| VDD | Pin2 或 Pin4, 5V | 资料写底板 VDD 为 3.6V~6V。若是裸模块 VCC，只能接 3.3V。 |
| GND | 任意 GND, 如 Pin6/9/14/20/25/30/34/39 | 必须共地。 |
| TXD | Pin10, UART4_RXD | 模块输出接板端接收。 |
| RXD | Pin8, UART4_TXD | 模块接收接板端输出。 |
| 1PPS | 首版不接 | 后续要授时或定位成功脉冲再接 GPIO。 |
| ON/OFF, VBAT | 首版不接 | ON/OFF 悬空；VBAT 后续可接备份电池提升热启动。 |

只在 EULER_40PEXP/40Pin 扩展板侧直连 3.3V TTL。不要绕过扩展板去接 SS928 SoC 原始 UART 管脚；原始 UART IO 可能是 1.8V。

## 板端启动

先确认板子网络不是走 VPN 虚拟网卡，并能正常 SSH 到 SS928。只读检查：

```sh
hostname
uname -a
ls -l /dev/ttyAMA4
bluetoothctl show
```

上传目录到板端，例如 `/root/dx_gp21_tracker`，然后运行：

```sh
cd /root/dx_gp21_tracker
sh ./start_ss928_gnss_track.sh
```

脚本会设置 UART4 pinmux：

```sh
bspmm 0x102F0134 0x1201  # Pin10 -> UART4_RXD
bspmm 0x102F0138 0x1201  # Pin8  -> UART4_TXD
```

## 最小验证

接线后先看 NMEA：

```sh
cd /root/dx_gp21_tracker
timeout 10 python3 ./dx_gp21_tracker.py --config config.ss928_uart4.json --no-ble --dump-nmea
```

应看到 `$GNGGA`、`$GNRMC`、`$GPTXT` 等文本。室内可能长时间没有有效定位，先靠窗或到室外等待。使用陶瓷/无源天线时出现 `ANTENNA OPEN` 不一定是故障。

本地或板端可先跑模拟模式：

```sh
python3 ./dx_gp21_tracker.py --simulate --once --no-ble --track-dir ./tmp_tracks
```

模拟会输出 `loc`、`tl`、`trk`、`ts` JSON 帧，形状和小程序 BLE 协议一致。

## BLE 小程序协议

广播名：`DX-GP21-Track`

UUID 复用 Nordic UART Service：

- Service: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX/write: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX/notify: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

小程序命令：

- `TL`：列出板端本地轨迹。
- `TG <i> <offset>`：按轨迹索引和偏移分页读取轨迹点。
- `TF 1` / `TF 0`：开启或关闭实时位置推送。
- `TS`：读取串口、定位、卫星数、轨迹数量等状态。

定位点保存为 WGS84；小程序轨迹工具会在中国大陆坐标范围内转换为 GCJ-02 用于微信地图显示。

## USB-TTL 备用

如果 40Pin UART4 被别的外设占用，可以先用 USB 转 TTL 调试：

| DX-GP21-A | USB-TTL |
| --- | --- |
| VDD | 5V 或按转接板/底板标注供电 |
| GND | GND |
| TXD | RXD |
| RXD | TXD |

USB-TTL 逻辑电平选 3.3V，插到 SS928 后把配置或参数改成 `/dev/ttyUSB0` 等实际设备：

```sh
python3 ./dx_gp21_tracker.py --serial-device /dev/ttyUSB0 --baud 115200 --dump-nmea --no-ble
```

## 微信云开发轨迹上传

`config.ss928_uart4.json` 默认启用现有 CloudBase telemetry 上传。程序只在
DX-GP21-A 产生有效 WGS84 定位、且本地 JSONL 已成功保存后，把单个轨迹点异步
提交到 `smartbag-device-ingest`；MT5710 只提供联网能力，不参与定位。

板端启动前通过服务环境设置 `SMARTBAG_UPLOAD_TOKEN`，不要把令牌写入配置或日志。
断网、超时或未配置令牌不会终止 GNSS、本地轨迹或 BLE。最新有效坐标会原子写入
`/tmp/smartbag_latest_location.json`，仅供最终摔倒事件在位置仍新鲜时引用。
