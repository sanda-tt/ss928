# SmartBag 板端 CloudBase 上传

## 目标

在不改变既有传感器判断和本地告警行为的前提下，让板端通过微信云开发现有统一
telemetry 接口上传 DX-GP21-A 轨迹、BMI270 处理后姿态与每日统计，以及最终摔倒信号，
并让真实小程序已有读取链路可以收到这些数据。

## 最终方案

- `work/smartbag_cloud_uploader/telemetry_client.py` 提供统一 payload builder 和有界后台上传客户端。
- DX-GP21-A 只在有效点成功写入本地轨迹后上传，并原子更新 `/tmp/smartbag_latest_location.json`。
- BMI270 上传现有 HUNCH 规则处理后的 `good` / `bad`；校准后角度只作诊断，不上传原始 IMU 数组。
- 每日姿态采用本地日期的绝对累计值，稳定写入 `posture_daily_stats/bag001_<date>`。
- 摔倒上传挂在既有最终 `FALL_ALARM` 分支之后；FREEFALL、IMPACT、pending、相机/MR20 预警均不上传。
- MT5710 仅提供网络传输，不作为 GNSS 数据源。
- `smartbag-device-ingest` 对 partial status 使用顶层 merge，避免 GNSS 与姿态互相覆盖。

## 关键文件

- `work/smartbag_cloud_uploader/telemetry_client.py`
- `work/dx_gp21_tracker/dx_gp21_tracker.py`
- `work/linux_bmi270_backpack/posture_cloud.py`
- `work/linux_bmi270_backpack/fall_fusion_runtime.py`
- `work/ssminiprogram/cloudfunctions/smartbag-device-ingest/lib/telemetry-core.js`
- `work/ssminiprogram/cloudfunctions/smartbag-device-ingest/index.js`
- `work/ssminiprogram/tools/simulate_device_upload.py`
- `skills/smartbag-cloud-upload/references/upload-contract.md`

## 验证结果

- Python 单元测试覆盖 payload、队列、DX-GP21-A 接入、姿态累计和最终摔倒 sink。
- Node 测试覆盖接口校验、partial merge、每日统计和小程序四类数据映射。
- `smartbag-device-ingest` 已通过 CloudBase MCP 更新并恢复 Active/Available。
- 三个模拟 HTTPS 请求均返回 HTTP 200。
- MCP 已在 `device_status`、`track_points`、`posture_daily_stats`、`alarm_history` 回读到本次记录。
- `smartbag-app-api` 四个读取 action 均成功返回对应记录，证明小程序服务层读取链路成立。

## 未验证边界

- 未连接或读取真实 DX-GP21-A、BMI270、MT5710。
- 未在板端启动服务、修改网络或启动项，也未做真实摔倒动作。
- 未使用微信真机验证最终页面刷新、BLE 或地图表现。
- 因而当前结论是源码、单测、模拟 HTTPS、云数据库和 Event Function 回读已验证，不能等同于物理硬件验证。

## 不要再走的路线

- 不要把 MT5710 当作 GNSS 模块；它只承载网络。
- 不要上传 BMI270 原始 IMU 流或绕过原有 HUNCH/摔倒触发规则。
- 不要把相机/MR20 预警混入 `alarm_history`；当前云端告警只接受 `fall_detected`。
- 不要用 full set 覆盖 `device_status`，否则轨迹位置和姿态字段会互相删除。
- 不要把上传成功描述为外设或板端服务已经工作。
