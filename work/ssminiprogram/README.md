# SS928 Smart Backpack Mini Program

## 板端统一 CloudBase telemetry 上传

板端通过现有微信云开发 HTTP 云函数 `smartbag-device-ingest` 上传，固定设备
ID 为 `bag001`。当前统一接口接收：

- DX-GP21-A 的有效 GNSS 轨迹点；MT5710 只负责网络传输；
- BMI270 已按现有 HUNCH 规则处理后的 `good` / `bad` 姿态、佩戴状态、提醒状态和每日绝对累计；
- 现有摔倒检测器最终确认的 `FALL_ALARM` / `fall_detected` 信号。

BMI270 的校准后 roll/pitch/yaw 仅作为诊断快照，原始加速度计和陀螺仪数组不上传。
相机和 MR20 的普通预警信号不上传。摔倒记录只在最终规则触发时写一次，并在
DX-GP21-A 位置缓存仍新鲜时附带位置。

本地只检查 payload、不访问云端：

```powershell
python .\tools\simulate_device_upload.py --kind all --dry-run
```

通过当前 CloudBase 接口分别上传轨迹、姿态/日统计和摔倒测试记录：

```powershell
python .\tools\simulate_device_upload.py --kind all
```

令牌按顺序从 `SMARTBAG_UPLOAD_TOKEN`、`UPLOAD_TOKEN` 或 Git 忽略的
`.local/device-access.md` 读取，脚本不会输出令牌。小程序仍通过
`smartbag-app-api` 的 `getRealtimePosture`、`getDailyPosture`、
`getTrackPoints` 和 `getAlarmHistory` 读取这些记录。

这个原生微信小程序用于连接 SS928 板端 BLE Nordic UART Service 外设：

- `DX-GP21-Track`：DX-GP21 GNSS 定位、实时位置和板端本地轨迹。
- `BMI270-Backpack`：BMI270 姿态数据和姿态监控。

## BLE 参数

- Service UUID：`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX/write UUID：`6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX/notify UUID：`6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

## GNSS 轨迹页

轨迹页扫描 `DX-GP21-Track`，支持以下板端命令：

- `TL`：读取轨迹列表。
- `TG <i> <offset>`：分页读取轨迹点。
- `TF 1` / `TF 0`：开启或关闭实时位置。
- `TS`：读取 GNSS、串口和轨迹状态。

板端每行发送一条 JSON。实时定位帧示例：

```json
{"typ":"loc","t":1719981296.0,"lat":31.23042,"lon":121.4737,"acc":0.9,"alt":42.0,"spd":0.8,"course":92.0,"fix":1,"sat":19,"src":"dx_gp21","cs":"wgs84"}
```

小程序会把 WGS84 坐标转换为微信地图使用的 GCJ-02 坐标。

## 云端轨迹测试

在 Windows PowerShell 中设置上传令牌后，可一次上传多条测试轨迹，供小程序“轨迹跟踪”页的“云端刷新”读取：

```powershell
$env:SMARTBAG_UPLOAD_TOKEN = "<比赛上传令牌>"
python .\tools\upload_track_test_points.py --count 5
```

脚本默认上传 `bag001` 的 5 个点；可用 `--latitude`、`--longitude`、`--step` 和 `--interval-seconds` 调整路线。先检查数据而不发送时，使用 `--dry-run`。若没有环境变量，脚本会读取 Git 忽略的 `.local/device-access.md` 中的 `SMARTBAG_UPLOAD_TOKEN`、`UPLOAD_TOKEN` 或“上传令牌”字段；令牌不会写入项目文件或脚本输出。

## 使用方式

1. 在 SS928 上按 `work/dx_gp21_tracker/README.md` 接好 DX-GP21-A 到 40Pin UART4。
2. 在板端运行 `sh ./start_ss928_gnss_track.sh`，确认 BLE 广播名为 `DX-GP21-Track`。
3. 用微信开发者工具打开本目录，真机预览。
4. 进入“儿童安全轨迹跟踪”，点击“扫描”，连接 `DX-GP21-Track`。
5. 点击“实时开”看实时位置，或点击“列表”加载板端本地轨迹。

注意：微信 BLE 需要真机验证，开发者工具模拟器不能替代真实手机蓝牙链路。
