---
name: smartbag-cloud-upload
description: Implement the SS928 smart safety backpack's minimal CloudBase upload path. Use when any board-side module needs to upload status, GNSS coordinates, IMU attitude, risk results, track points, or alarm events such as falls and collision warnings to the deployed smartbag-device-ingest HTTP Function. Also use when modifying upload helpers, simulator scripts, or event-triggered reporting code. Preserve existing BLE and local alert behavior; do not redesign the backend or add HMAC, nonce, user binding, or other non-MVP mechanisms.
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

## Minimal workflow

1. Find the point where the module confirms a valid event or has a new sample to upload.
2. Reuse or add one small HTTP helper instead of embedding request logic throughout the detection code.
3. Build one telemetry JSON object using the contract in `references/upload-contract.md`.
4. Send the request with a short timeout, normally 5 seconds.
5. On upload failure, log one concise error and continue the local detection loop.
6. Do not change BLE broadcasting, local vibration, sound, LED, or the original detection algorithm unless the user explicitly asks.

## Event mapping

Use these destinations:

- Current device snapshot -> `status` -> updates `device_status`
- GNSS history point -> `trackPoints` -> appends to `track_points`
- Fall, collision, danger, or other event -> `alarms` -> appends to `alarm_history`

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

At the final confirmed-fall branch, call a helper shaped like:

```python
report_fall_event(latitude, longitude, roll, pitch, yaw)
```

Use:

```text
alarmType = fall_detected
riskLevel = 3
message = 检测到用户摔倒
```

The complete example is in `references/upload-contract.md`.

## Completion checklist

Before declaring the integration done, verify only these items:

- The request URL and `X-Upload-Token` header are correct.
- The payload matches the deployed validator.
- A failed network request does not terminate the sensor or detection process.
- One manual test creates the expected `device_status`, `track_points`, or `alarm_history` record.
- Existing BLE and local alerts still run.
