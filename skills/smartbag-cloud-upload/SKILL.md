---
name: smartbag-cloud-upload
description: Implement the SS928 smart safety backpack's minimal CloudBase upload path. Use when a board-side module needs to upload DX-GP21-A GNSS tracks, processed BMI270 posture, daily posture totals, or final fall alarms to the deployed smartbag-device-ingest HTTP Function. Also use when modifying upload helpers, simulator scripts, or event-triggered reporting code. Preserve existing BLE and local alert behavior; do not redesign the backend or add HMAC, nonce, user binding, or other non-MVP mechanisms.
---

# Smartbag Cloud Upload

## Goal

Connect an existing SS928 detection or sensor module to the already deployed CloudBase upload endpoint with the least code and configuration.

The fixed flow is:

```text
SS928 module -> HTTPS POST -> smartbag-device-ingest -> CloudBase collections
```

## Source of truth

Before writing code, read these files if they exist in the repository:

- `work/ssminiprogram/cloudfunctions/smartbag-device-ingest/lib/telemetry-core.js`
- `work/ssminiprogram/tools/simulate_device_upload.py`
- `work/ssminiprogram/tools/upload_track_test_points.py`
- `work/ssminiprogram/cloudfunctions/smartbag-device-ingest/index.js`

Treat the deployed function and these files as canonical. Do not invent a second protocol. If the local code differs from this skill, follow the local code and update this skill later.

## Fixed connection

- Method: `POST`
- URL: `https://cloud1-d7gdmg27139f4fbf2.service.tcloudbase.com/smartbag-device-ingest/v1/device/telemetry`
- Headers:

```text
Content-Type: application/json
X-Upload-Token: <比赛上传令牌>
```

Read the token from an environment variable or existing project configuration. Never create another authentication scheme.

Recommended board-side environment variable:

```text
SMARTBAG_UPLOAD_TOKEN
```

For Windows local testing, `upload_track_test_points.py` also accepts legacy
`UPLOAD_TOKEN`, then falls back to the Git-ignored
`.local/device-access.md`. Never copy a token into source, a skill, a log, or
a commit.

## HTTP Function Runtime Contract

`smartbag-device-ingest` is a CloudBase **HTTP** Node service, not a WeChat
Event Function. Its database layer must use `@cloudbase/node-sdk`:

```javascript
const tcb = require("@cloudbase/node-sdk");
const app = tcb.init({ env: process.env.CLOUDBASE_ENV_ID || "cloud1-d7gdmg27139f4fbf2" });
const db = app.database();
```

Do not use `wx-server-sdk` in this HTTP service. Keep `wx-server-sdk` in
`smartbag-app-api`, which is the Mini Program Event Function.

Node SDK database methods receive documents directly. Do not use the WeChat
SDK wrapper `{ data: document }`:

```javascript
await db.collection("track_points").add(document);
await db.collection("device_status").doc(documentId).set(dataWithoutId);
```

When setting a known document ID, remove `_id` from the `set()` data and pass
it only to `doc(documentId)`.

## Track Idempotency Contract

`track_points` has a unique `(deviceId, requestId)` index. Every point in a
single upload needs its own non-empty `requestId`; a top-level request ID alone
is insufficient. A duplicate or omitted point ID can return a database error
after the status document has already been updated.

Use a stable device-generated ID for real telemetry. The local simulator uses
`track-point-<timestamp>-<index>`:

```json
{
  "requestId": "track-point-1784262534929-0",
  "reportedAt": 1784262494929,
  "location": { "latitude": 30.65736, "longitude": 104.0832, "valid": true }
}
```

Do not retry a failed multi-point request blindly. First inspect whether
`device_status` or a prefix of the point list was already written; retrying
with the same IDs is safe only when the function intentionally treats duplicate
IDs as success.

## Minimal workflow

1. Find the point where the module confirms a valid event or has a new sample to upload.
2. Reuse or add one small HTTP helper instead of embedding request logic throughout the detection code.
3. Build one telemetry JSON object with the shared
   `work/smartbag_cloud_uploader/telemetry_client.py` builders and the contract
   in `references/upload-contract.md`.
4. Send the request with a short timeout, normally 5 seconds.
5. On upload failure, log one concise error and continue the local detection loop.
6. Do not change BLE broadcasting, local vibration, sound, LED, or the original detection algorithm unless the user explicitly asks.

For a local GNSS history proof, run on `LOCAL-WIN`:

```powershell
python .\tools\upload_track_test_points.py --count 5 --dry-run
python .\tools\upload_track_test_points.py --count 5
```

The second command must return `HTTP 200` before claiming upload success.

## CloudBase MCP Audit and Deployment

Use this order before changing live resources:

1. `auth(action:"status")` and `envQuery(action:"info")`; confirm the full EnvId.
2. List `smartbag-device-ingest` and `smartbag-app-api`; confirm the former is HTTP and the latter is Event.
3. List `device_status`, `track_points`, and `alarm_history`; verify their permissions remain `ADMINONLY`.
4. Inspect function logs and the active function configuration before diagnosing an HTTP 500.
5. Deploy `smartbag-device-ingest` with `manageFunctions(updateFunctionCode)` and `functionRootPath` set to the **parent** `work/ssminiprogram/cloudfunctions` directory, not the individual function folder.
6. Re-run a one-point upload, then read `track_points` through MCP and invoke `smartbag-app-api` with `getTrackPoints`.

The HTTP deployment package must include a top-level `scf_bootstrap` file with
LF line endings and Unix executable mode `755`; it starts the Node process on
port `9000`. A Windows workspace does not prove that mode survived packaging,
so confirm the deployed HTTP endpoint with a real request.

## Failure Triage

| Evidence | Meaning | Next action |
|---|---|---|
| `401 INVALID_UPLOAD_TOKEN` | HTTP route is live; credential is missing or wrong. | Check only the local credential source and function environment variable. |
| `404 NOT_FOUND` | Function route/path is wrong. | Keep `/v1/device/telemetry` inside the function and the function-name gateway prefix outside it. |
| `500 STATUS_WRITE_FAILED` | Status document failed before any points were appended. | Check Node SDK initialization, direct `set(document)` API shape, and omit `_id` from set data. |
| `500 TRACK_WRITE_FAILED` with duplicate-key error | A track point has a repeated or missing `requestId`. | Generate a unique ID per point; inspect existing writes before retrying. |
| `500 INTERNAL_ERROR` | The handler hid the source exception or stale code is deployed. | Add safe operation-level error codes, redeploy from the parent cloudfunctions root, then inspect logs. |
| `HTTP 200` but page is empty | Upload worked but read path is unproven. | Invoke `smartbag-app-api:getTrackPoints`, then refresh the Mini Program track page. |

Never return stack traces, request bodies, or tokens from the public HTTP
endpoint. Operation-level codes such as `STATUS_WRITE_FAILED` are sufficient
for diagnosis.

## Event mapping

Use these destinations:

- Current device snapshot -> `status` -> updates `device_status`
- GNSS history point -> `trackPoints` -> appends to `track_points`
- Final `FALL_ALARM` only -> `alarms` -> appends to `alarm_history`
- Processed posture daily totals -> `postureDaily` -> overwrites `posture_daily_stats/bag001_<date>`

One upload may contain all three.

## Required rules

- Keep `deviceId` fixed as `bag001` for the competition demo.
- Use Unix time in milliseconds for `reportedAt`.
- Use the latest valid GNSS coordinates when available.
- If location is unavailable, send no location or an empty location object according to the current validator. Do not crash the detector.
- Upload one confirmed event once. Do not send the same fall continuously on every loop iteration.
- Keep the implementation synchronous only if it does not block the detection loop noticeably; otherwise send in a short background task.
- Do not add HMAC, nonce, replay ledgers, accounts, user binding, or extra cloud functions.

## Fall detection integration

At the final confirmed-fall branch, pass the existing event to a helper shaped like:

```python
report_fall(event, fresh_dx_gp21_location)
```

Use:

```text
alarmType = fall_detected
riskLevel = 3
message = 检测到用户摔倒
```

Reject any event whose `signal` is not `FALL_ALARM` or whose `alarmType` is
not `fall_detected`. Camera/MR20 warnings remain local and are not part of this
upload contract. The complete example is in `references/upload-contract.md`.

## Completion checklist

Before declaring the integration done, verify only these items:

- The request URL and `X-Upload-Token` header are correct.
- The payload matches the deployed validator.
- Each `trackPoints` item has a unique `requestId`.
- A failed network request does not terminate the sensor or detection process.
- A real local simulator request returns HTTP 200 and MCP confirms the expected
  `device_status`, `track_points`, `posture_daily_stats`, and `alarm_history` records.
- `smartbag-app-api` returns the uploaded records through `getRealtimePosture`,
  `getDailyPosture`, `getTrackPoints`, and `getAlarmHistory`.
- Existing BLE and local alerts still run.
