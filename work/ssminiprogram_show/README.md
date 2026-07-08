# SS928 Smart Backpack Mini Program

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

## 使用方式

1. 在 SS928 上按 `work/dx_gp21_tracker/README.md` 接好 DX-GP21-A 到 40Pin UART4。
2. 在板端运行 `sh ./start_ss928_gnss_track.sh`，确认 BLE 广播名为 `DX-GP21-Track`。
3. 用微信开发者工具打开本目录，真机预览。
4. 进入“儿童安全轨迹跟踪”，点击“扫描”，连接 `DX-GP21-Track`。
5. 点击“实时开”看实时位置，或点击“列表”加载板端本地轨迹。

注意：微信 BLE 需要真机验证，开发者工具模拟器不能替代真实手机蓝牙链路。
