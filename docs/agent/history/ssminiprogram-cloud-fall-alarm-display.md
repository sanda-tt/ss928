# 小程序 CloudBase 摔倒报警历史

## 目标

将智能安全书包小程序的报警历史入口接入 CloudBase，只显示 `alarmType: "fall_detected"` 的摔倒记录，不影响既有 BLE 页面和功能。

## 最终方案

- `pages/placeholder/index.js` 在 `onShow` 调用 `services/cloud-data-source.js` 的 `getAlarmHistory()`。
- 页面不直接读 `alarm_history` 集合；调用链保持为 `页面 -> cloud-data-source.js -> smartbag-app-api -> CloudBase`。
- 过滤摔倒报警并按 `reportedAt`（兼容 `deviceTimestampMs`、`receivedAt`）倒序；映射为显示用的标题、时间、高风险、定位和消息字段。
- 无定位时显示“暂无定位”，不渲染地图；空记录和读取失败均保留可用页面状态。
- 不再为该云端报警页填充固定演示记录。

## 关键文件

- `work/ssminiprogram/miniprogram/pages/placeholder/index.js`
- `work/ssminiprogram/miniprogram/pages/placeholder/index.wxml`
- `work/ssminiprogram/miniprogram/pages/placeholder/index.wxss`
- `work/ssminiprogram/miniprogram/services/cloud-data-source.js`
- `work/ssminiprogram/cloudfunctions/smartbag-app-api/index.js`
- `work/ssminiprogram/tools/upload_fall_test_alarms.py`

## 验证结果

- CloudBase MCP `auth(status)`：环境 `cloud1-d7gdmg27139f4fbf2` 为 `READY`，与小程序初始化环境一致。
- `node --check` 通过页面与数据服务语法检查。
- `node tests/alarm-history-page.test.js` 验证仅显示摔倒记录、按时间倒序、无定位与失败状态。
- `node tests/smartbag-cloud.test.js` 通过。
- 未在微信开发者工具或真机调用已部署 Event Function；仍需用真实 `fall_detected` 上传记录验证页面渲染。

## 本地上传测试

- `upload_fall_test_alarms.py --count 3` 会通过已部署的 `smartbag-device-ingest` HTTP 函数生成本地测试摔倒记录；令牌只从环境变量或 Git 忽略的 `.local/device-access.md` 读取。
- 每条报警必须带唯一 `requestId`，因为 `alarm_history` 的 `(deviceId, requestId)` 为唯一索引；仅有 `alarmId` 会导致同一批的后续写入失败。
- 2026-07-17 已验证脚本返回 HTTP 200，MCP 确认 3 条新 `fall_detected` 记录已写入 `alarm_history`。

## 注意事项

- 保持 `smartbag-app-api` 当前的 100 条历史读取上限；需要更多记录时，服务端和页面同时添加游标分页。
- 旧 `tests/alarm-utils.test.js` 的固定演示数据顺序断言与现有演示数据顺序不一致；该测试与当前云端报警页无依赖关系，未修改。
