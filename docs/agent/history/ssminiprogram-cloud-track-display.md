# SS928 小程序云端历史轨迹显示

## 目标

在 `work/ssminiprogram` 的轨迹跟踪页显示 CloudBase 中已上传的历史轨迹，同时保留原有 BLE 扫描、板端轨迹和实时定位功能。

## 最终方案

- 页面在 `onShow` 和“云端刷新”按钮中调用 `services/cloud-data-source.js` 的 `getTrackPoints()`。
- 仅通过已部署的 `smartbag-app-api` Event Function 读取数据，不在页面直接访问业务集合。
- 将 CloudBase 的 `reportedAt`、`location`、`speed`、`heading` 映射为地图点；坐标复用 `track-utils.js` 的 WGS84 到 GCJ-02 转换，按时间正序绘制折线。
- 页面显示加载中、空轨迹和失败状态；BLE 连接和既有命令没有移除。

## 关键文件

- `work/ssminiprogram/miniprogram/pages/tracks/index.js`
- `work/ssminiprogram/miniprogram/pages/tracks/index.wxml`
- `work/ssminiprogram/miniprogram/utils/track-utils.js`
- `work/ssminiprogram/miniprogram/services/cloud-data-source.js`

## 验证结果

- `node --check miniprogram/pages/tracks/index.js`
- `node tests/track-utils.test.js`
- `node tests/smartbag-cloud.test.js`
- `node tests/home-ui.test.js`

均通过；未在微信开发者工具或真机验证已部署云函数的实际返回和地图渲染。

2026-07-17 CloudBase MCP 审计与上传验证：已确认环境、HTTP/Event 函数和 NoSQL 集合可用；`upload_track_test_points.py --count 5` 返回 HTTP 200，`track_points` 新增 5 条带唯一 `requestId` 的记录，`smartbag-app-api` 的 `getTrackPoints` 返回这些记录。小程序源码读取链路已验证到 Event Function 返回；仍待微信开发者工具或真机确认实际地图渲染。

## 注意事项

- 云端历史读取必须继续走 `cloud-data-source.js -> smartbag-app-api`，不要在页面调用 `wx.cloud.database()`。
- `getTrackPoints()` 现有接口最多返回 100 条记录且为接收时间倒序；页面需在显示前按轨迹时间正序排序。
- 本地上传验证可使用 `work/ssminiprogram/tools/upload_track_test_points.py`；令牌优先从 `SMARTBAG_UPLOAD_TOKEN`（或兼容的 `UPLOAD_TOKEN`）读取，未设置时读取 `.local/device-access.md`，不得写入源码、日志或提交记录。
