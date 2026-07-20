# 智能安全书包板端统一 Telemetry 上传设计

日期：2026-07-20

## 1. 目标与边界

在不连接板端外设的前提下，完成 SS928 板端到现有 CloudBase 链路的代码集成，使微信小程序能够读取：

- DX-GP21-A 提供的 GNSS 历史轨迹和最新位置；
- BMI270 已处理的实时姿态状态和每日姿态累计；
- 现有融合规则最终确认的摔倒事件。

MT5710 只作为 5G RedCap 网络承载，不提供 GNSS 数据。摄像头和 MR20 的普通预警不上传；它们只继续作为本地避险输出及既有摔倒复核规则的输入。

本阶段不连接、探测或操作板端外设，不修改持久网络配置，不启动或重启板端服务。完成标准限于源代码、自动化测试、本地模拟、CloudBase 接口部署与数据读回；实物传感器和蜂窝链路验证保留为待验证项。

## 2. 现有约束

- 设备固定为 `bag001`。
- 板端继续向现有 HTTPS 端点发送 `POST /smartbag-device-ingest/v1/device/telemetry`。
- 鉴权继续使用环境变量 `SMARTBAG_UPLOAD_TOKEN` 和请求头 `X-Upload-Token`，不新增 HMAC、nonce、账户或设备绑定。
- 写链路保持 `板端 -> smartbag-device-ingest -> CloudBase`。
- 读链路保持 `小程序页面 -> cloud-data-source.js -> smartbag-app-api -> CloudBase`。
- 保留现有 BLE、本地轨迹、灯光、振动、语音、告警日志和检测算法。
- 每个 `track_points` 和 `alarm_history` 文档都必须携带非空、唯一的 `requestId`。

## 3. 方案选择

采用“扩展现有统一 telemetry 接口”。板端各现有进程复用一个轻量上传模块，分别生成同一协议中的轨迹、姿态和摔倒数据。现有 HTTP Function 扩展为局部合并最新状态，并写入每日姿态累计。小程序继续复用现有读取接口，不创建第二套协议或第二个上传函数。

不采用以下方案：

- 各模块维护独立 HTTP 接口：会重复鉴权、重试和字段转换，并增加协议漂移风险。
- 新建集中采集守护进程：需要新增进程间通信和服务编排，超出当前未接外设阶段的最小需求。

## 4. 组件设计

### 4.1 共享上传模块

在 `work/` 下建立可被 DX-GP21-A 与 BMI270 程序复用的小型 Python 模块，职责仅包括：

- 构建 telemetry v1 JSON；
- 从环境变量读取令牌；
- 使用 Python 标准库发送 HTTPS POST，避免增加大型依赖；
- 使用短超时和后台工作线程，避免阻塞 GNSS 与 50 Hz IMU 循环；
- 对最新姿态采用“只保留待发最新值”，对轨迹和摔倒事件采用有界队列；
- 网络失败时输出不含令牌和完整负载的简短日志，并继续本地业务；
- 为轨迹点和摔倒事件生成稳定且唯一的 `requestId`。

上传默认由配置显式启用。未设置令牌时只记录一次跳过原因，不终止检测进程。

### 4.2 DX-GP21-A 轨迹集成

只在 `NmeaLocationTracker` 已确认有效定位、且 `TrackStore.append()` 成功后上传。数据源是 DX-GP21-A 的 WGS84 定位，不读取 MT5710 的串口或 GNSS 命令。

每个轨迹点写入：

```json
{
  "requestId": "track-dxgp21-<reportedAt>-<suffix>",
  "reportedAt": 1784160000000,
  "location": {
    "latitude": 30.6701,
    "longitude": 104.0602,
    "accuracyM": 8.5,
    "valid": true
  },
  "speed": 1.2,
  "heading": 92.0,
  "altitudeM": 42.0,
  "source": "dx_gp21",
  "coordinateSystem": "wgs84"
}
```

同一次请求还携带 `status.location`，用于更新小程序最新位置。板端原有 JSONL 轨迹和 BLE 实时位置保持不变。

DX-GP21-A 同时原子更新一个本地“最新有效位置”缓存文件。摔倒事件只在该位置仍处于配置的新鲜度窗口内时引用它；缓存不存在、损坏或过期时省略位置，不影响摔倒上传。

### 4.3 BMI270 姿态集成

小程序现有 `getRealtimePosture` 接口读取 `device_status/bag001`，消费的是处理后的姿态状态，而不是高频原始 IMU 数组。因此 BMI270 上传以处理结果为主：

```json
{
  "reportedAt": 1784160000000,
  "posture_status": "good",
  "is_wearing": true,
  "reminder_active": false,
  "attitude": {
    "rollDeg": 1.2,
    "pitchDeg": -3.4,
    "yawDeg": 92.1
  }
}
```

规则如下：

- `bad` 复用现有 HUNCH 判定：校正后 pitch 小于当前阈值、满足运动门控并持续达到现有保持时间；不使用单帧角度直接判定。
- `good` 表示当前收到连续有效 BMI270 样本且 HUNCH 持续判定未成立。
- `reminder_active` 与已成立的 HUNCH 状态一致。
- `attitude` 保存安装校准后的 roll、pitch、yaw，供诊断和后续扩展；小程序当前不展示角度。
- 不上传 `ax/ay/az`、`gx/gy/gz`、原始数组或 50 Hz 样本流。
- 姿态快照按配置的低频周期上传，默认设计值为 5 秒一次。

当前代码没有独立的佩戴检测传感器。首版在收到连续有效 BMI270 样本时设置 `is_wearing: true`，停止收到有效快照后由小程序的超时规则显示离线，而不声称已物理验证“正在佩戴”。后续若增加肩带压力或其他佩戴传感器，再替换该字段来源。

为避免现有检测器的冷却事件被误当作持续状态，将在不改变阈值与告警输出的前提下，为 `AnomalyDetector` 暴露当前 HUNCH 是否已满足保持条件的只读状态。

### 4.4 每日姿态累计

小程序现有 `getDailyPosture(date)` 读取 `posture_daily_stats`，字段为：

```json
{
  "device_id": "bag001",
  "date": "2026-07-20",
  "wearing_seconds": 10800,
  "good_seconds": 8100,
  "bad_seconds": 2700,
  "updated_at": 1784160000000
}
```

累计由板端 BMI270 进程维护为绝对值，并原子保存到本地 JSON 状态文件：

- 仅在连续有效采样期间累计；
- `good_seconds + bad_seconds` 不超过 `wearing_seconds`；
- 日期变化时建立新日期记录；
- 进程重启后从本地状态恢复；
- 上传失败不丢失累计；
- 重试发送绝对值而不是增量，避免重复请求造成重复计时。

telemetry 负载新增可选的 `postureDaily` 对象。HTTP Function 按 `device_id + date` 的稳定文档 ID 覆盖写入，保留现有唯一索引语义。

### 4.5 摔倒事件集成

只在 `FallFusionRuntime` 已产生最终事件时上传，即现有代码中的：

```text
signal = FALL_ALARM
alarmType = fall_detected
```

不修改现有融合条件、阈值或时间窗口。普通摄像头/MR20 预警、BMI270 的 IMPACT/FREEFALL 中间事件和待确认状态均不上传。

云端报警文档格式：

```json
{
  "requestId": "fall-<deviceTimestampMs>-<suffix>",
  "alarmId": "fall-<deviceTimestampMs>-<suffix>",
  "alarmType": "fall_detected",
  "riskLevel": 3,
  "message": "检测到用户摔倒",
  "reportedAt": 1784160000000,
  "location": {
    "latitude": 30.6701,
    "longitude": 104.0602,
    "valid": true
  },
  "details": {
    "signal": "FALL_ALARM",
    "conditions": {
      "imu_fall_confirmed": true,
      "recent_level_3_or_4_warning": true,
      "no_standing_recovery_7s": true
    }
  }
}
```

`location` 仅在最新 DX-GP21-A 定位有效且未过期时存在。最终事件继续写本地 JSONL；云上传作为附加 sink，上传失败不会影响本地日志。一个最终事件只入队一次。

### 4.6 CloudBase HTTP Function

扩展 `smartbag-device-ingest`，继续处理同一 telemetry v1 请求：

- `status`：局部合并到 `device_status/bag001`，防止 GNSS 更新覆盖姿态、姿态更新覆盖位置；
- `trackPoints`：追加到 `track_points`；
- `alarms`：追加到 `alarm_history`；
- `postureDaily`：覆盖写入 `posture_daily_stats` 的稳定日期文档。

服务端验证：

- `reportedAt` 必须是合理的 Unix 毫秒时间；
- 经纬度必须有限且处于合法范围；
- roll、pitch、yaw 必须是有限数；
- `posture_status` 只允许 `good` 或 `bad`；
- 每日秒数必须为非负有限数并满足累计关系；
- 轨迹和报警的 `requestId` 必须非空；
- 仍限制 64 KiB 请求体，不返回堆栈、令牌或完整请求体。

现有 `X-Upload-Token`、HTTP 路由、函数类型、端口和集合权限不改变。

### 4.7 小程序读取

现有读取接口已经覆盖目标数据：

- 轨迹页调用 `getTrackPoints()`；
- 实时姿态页调用 `getRealtimePosture()`；
- 每日姿态页调用 `getDailyPosture(date)`；
- 摔倒历史页调用 `getAlarmHistory()` 并过滤 `fall_detected`。

实现优先复用现有页面和 `cloud-data-source.js`。只有测试证明现有字段映射不完整时才做最小修正，不改 BLE 通道、不新增数据库直读。

## 5. 数据流

```text
DX-GP21-A -> dx_gp21_tracker -> shared uploader -> status.location + trackPoints
BMI270 -> posture correction/HUNCH -> shared uploader -> status posture + postureDaily
BMI270 + recent local high warning + no recovery -> FALL_ALARM
                                                -> shared uploader -> alarms

shared uploader -> HTTPS -> smartbag-device-ingest -> CloudBase collections
CloudBase collections -> smartbag-app-api -> cloud-data-source.js -> Mini Program pages
```

MT5710 只为 HTTPS 请求提供网络接口和默认路由，不参与任何定位字段生成。

## 6. 错误处理与离线行为

- 无令牌、DNS 失败、超时、HTTP 非 200 或服务端错误均不得终止传感器循环。
- 姿态快照允许用较新的快照替换尚未发送的旧快照。
- 轨迹和摔倒事件使用有界队列；队列满时必须记录计数和简短告警，不可无限增长内存。
- 本地轨迹、每日累计和摔倒 JSONL 先于或独立于网络上传存在，网络失败不删除本地证据。
- 不盲目重放可能已部分写入的多条批次；首版每次轨迹/报警请求只包含一个追加型记录。
- CloudBase 状态合并失败、轨迹写入失败、姿态累计写入失败和报警写入失败返回区分明确的安全错误码。

## 7. 测试与验证

实施严格采用测试先行：先添加失败测试并观察预期失败，再写最小实现。

本地测试覆盖：

- GNSS 点到 telemetry 轨迹格式的映射与唯一 ID；
- 无效定位不上传；
- 姿态 `good/bad` 使用持续 HUNCH 状态而非瞬时角度；
- 不包含原始 IMU 数组；
- 每日累计、日期切换、进程恢复和绝对值重试；
- 只有最终 `FALL_ALARM` 生成云报警；
- 新鲜、过期和缺失 GNSS 位置的摔倒格式；
- 网络失败不影响原有处理循环；
- HTTP Function 对局部状态的合并行为；
- `postureDaily` 写入和输入验证；
- 现有轨迹、姿态、报警小程序测试不回归。

CloudBase 验证顺序：

1. 本地所有相关 Python 和 Node 测试通过；
2. 使用 dry-run 输出三类脱敏负载；
3. 仅部署有变更的 CloudBase 函数；
4. 模拟上传轨迹、姿态、每日累计和摔倒事件并获得 HTTP 200；
5. 使用 CloudBase MCP 确认四个集合中的目标记录；
6. 通过 MCP 调用 `smartbag-app-api` 的四个读取 action，确认新数据可返回；
7. 运行小程序源代码与映射测试。

不把上述验证描述为 DX-GP21-A、BMI270、MT5710 或微信真机已经通过。实物接线后仍需分别验证真实 NMEA、真实姿态分类、真实摔倒触发、5G 联网和小程序真机显示。

## 8. 预期修改范围

- 新增共享板端 telemetry 上传模块及测试；
- 修改 `work/dx_gp21_tracker` 的配置、集成点、测试和 README；
- 修改 `work/linux_bmi270_backpack` 的配置、姿态状态/累计集成、摔倒 sink、测试和 README；
- 修改 `work/ssminiprogram/cloudfunctions/smartbag-device-ingest` 的协议处理、数据库写入和测试；
- 按测试结果最小修改 `work/ssminiprogram` 的模拟上传工具或字段映射；
- 更新 `docs/agent/history/` 对应记录和索引。

不修改摄像头/MR20 普通预警上传行为，不重构无关 BLE 或 UI，不删除原始资料，不操作板端服务和网络。
