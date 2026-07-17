---
name: smartbag-cloud-read
description: Connect WeChat Mini Program pages in the SS928 smart safety backpack project to data already stored in CloudBase. Use when a page needs to display the latest device status, GNSS track history, alarm history, or fall records through smartbag-app-api and miniprogram/services/cloud-data-source.js. Keep the existing BLE path intact, do not access ADMINONLY collections directly, and do not add login, binding, or non-MVP architecture.
---

# Smartbag Cloud Read

## Goal

Display CloudBase data in the existing WeChat Mini Program with the least change.

The fixed flow is:

```text
Mini Program page -> cloud-data-source.js -> smartbag-app-api -> CloudBase collections
```

## Source of truth

Before changing a page, read:

- `work/ssminiprogram/cloudfunctions/smartbag-app-api/index.js`
- `work/ssminiprogram/cloudfunctions/smartbag-app-api/lib/app-api-core.js`
- `work/ssminiprogram/miniprogram/services/cloud-data-source.js`
- The target page's `.js`, `.wxml`, `.wxss`, and `.json` files

Do not copy database logic into pages. Reuse the existing service layer.

## Runtime Boundary

Keep the two CloudBase runtimes separate:

| Component | Function type | SDK / responsibility |
|---|---|---|
| `smartbag-device-ingest` | HTTP Node service | `@cloudbase/node-sdk`; writes status, points, and alarms. |
| `smartbag-app-api` | Mini Program Event Function | `wx-server-sdk`; reads `ADMINONLY` collections for the Mini Program. |
| `cloud-data-source.js` | Native Mini Program client | calls `wx.cloud.callFunction`; never reads business collections directly. |

A successful HTTP upload does not itself prove the page can read data. The
required proof chain is: upload returns HTTP 200 -> MCP confirms the document
exists -> `smartbag-app-api:getTrackPoints` returns it -> the page maps it
without crashing.

## Available read operations

Use only these methods from `cloud-data-source.js`:

```javascript
getLatestStatus()
getTrackPoints()
getAlarmHistory()
```

They call the deployed Event Function:

```text
smartbag-app-api
```

with one of these actions:

```text
getLatestStatus
getTrackPoints
getAlarmHistory
```

## Minimal workflow

1. Identify which cloud data the page needs.
2. Import `cloud-data-source.js` using the existing project path style.
3. Load data in `onShow` so returning to the page refreshes the view.
4. Store only display-ready data in `setData`.
5. Show a simple empty state when no records exist.
6. On failure, keep the page usable and show one concise message.
7. Preserve existing BLE behavior unless the user explicitly asks to replace it.

For a historical track, `getTrackPoints()` returns documents in server
`receivedAt` descending order. Convert each CloudBase document from
`reportedAt`, `location`, `speed`, and `heading` into the existing map-point
shape, then sort by `reportedAt` ascending before building the polyline. Reuse
`track-utils.normalizeCloudTrackPoint()` so WGS84 points are converted for the
WeChat map; do not redraw a second coordinate conversion in the page.

Cloud status is a latest-state document and is overwritten on each successful
upload. `track_points` is append-only. Do not treat a new status document as
evidence that historical points were written.

## Page mapping

- Latest posture, risk, location, battery -> `getLatestStatus()`
- Historical route -> `getTrackPoints()`
- Falls, collisions, warnings -> `getAlarmHistory()`

The detailed field mapping and page examples are in `references/read-display-guide.md`.

## Alarm page rules

For `alarmType === "fall_detected"`, display:

```text
标题：检测到摔倒
时间：reportedAt 转换为本地年月日时分秒
风险等级：高风险
位置：latitude, longitude；无定位时显示“暂无定位”
消息：message
```

Sort newest first. Do not generate fixed fake records after the cloud integration is enabled.

## Required rules

- Do not call `wx.cloud.database()` from the page for these business collections.
- Do not change collection permissions from `ADMINONLY`.
- Do not add login, OPENID binding, device binding, or account pages.
- Keep the demo device fixed as `bag001` unless the existing service already hides that detail.
- Do not redesign the whole page. Modify only loading, mapping, empty state, error state, and required display fields.
- Do not remove BLE live data. CloudBase is mainly for remote status, history, and alarm records.
- Preserve the current `HISTORY_LIMIT` of 100. If the product needs more, add
  cursor pagination in both `smartbag-app-api` and the page; never silently
  claim an unbounded route was downloaded.
- A cloud call returning `items: []` is an empty state, not a reason to insert
  fake tracks. Check the fixed device ID, the upload HTTP result, and the Event
  Function response before changing UI code.

## MCP Read-Path Audit

When CloudBase MCP is available, audit in this order:

1. Confirm `auth(action:"status")` and the full active EnvId.
2. Read `track_points` for `deviceId: "bag001"` after a real simulator upload.
3. Invoke `smartbag-app-api` with `{ action: "getTrackPoints" }` and confirm
   the new `requestId` values appear in `items`.
4. Only then use WeChat Developer Tools or a real device to verify the map UI.

This distinguishes upload/database failures from Event Function failures and
from a page-only mapping/rendering problem.

## Failure Triage

| Symptom | Cause to check first | Correct action |
|---|---|---|
| HTTP upload is not 200 | Uploader or HTTP function failed. | Use `smartbag-cloud-upload`; do not change the page. |
| Database has points, but Event Function returns empty items | Wrong environment, device ID, collection query, or undeployed Event Function. | Audit `smartbag-app-api` and its function logs. |
| Event Function returns items, but map is empty | Page mapping/order/coordinate issue. | Reuse `normalizeCloudTrackPoint`, sort chronologically, and show the page error/empty state. |
| `CLOUD_CALL_FAILED` | Client has no CloudBase init, wrong function name, or Event Function error. | Verify `app.js`, `cloud-data-source.js`, and invoke the Event Function through MCP. |
| Old points only | Track query is ordered by `receivedAt` descending and limited to 100. | Check the latest uploaded `requestId`, then implement cursor paging only if needed. |

## Completion checklist

Verify only these items:

- The page calls the correct method from `cloud-data-source.js`.
- `wx.cloud.callFunction` reaches `smartbag-app-api` through the service layer.
- A record uploaded by the simulator appears on the target page.
- MCP confirms the same uploaded `requestId` is returned by
  `smartbag-app-api:getTrackPoints`.
- Empty and failed requests do not crash the page.
- Existing BLE pages and functions still compile.
