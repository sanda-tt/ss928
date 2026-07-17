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

## Completion checklist

Verify only these items:

- The page calls the correct method from `cloud-data-source.js`.
- `wx.cloud.callFunction` reaches `smartbag-app-api` through the service layer.
- A record uploaded by the simulator appears on the target page.
- Empty and failed requests do not crash the page.
- Existing BLE pages and functions still compile.
