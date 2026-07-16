---
name: wechat-cloudbase-iot
description: Develop and review native WeChat Mini Program + Tencent CloudBase IoT features for an SS928 Linux device, including BLE and cloud dual-channel data access, CloudBase initialization, HTTP/Event Functions, document-database collections, device upload APIs, OPENID-based user authorization, GNSS tracks, IMU posture, risk alarms, deployment, and debugging. Use when working under work/ssminiprogram or when requests mention SS928, smart backpack, wx.cloud, CloudBase, 云开发, 云函数, 云数据库, device telemetry, remote tracks, alarms, or device-to-mini-program networking.
---

# WeChat CloudBase IoT

Build the cloud path for the SS928 smart-backpack project without weakening or replacing the existing BLE path. Treat BLE and network access as two required, independent capabilities.

## Required reading order

1. Read the repository instructions and the current code under `work/ssminiprogram` before proposing changes.
2. Read the existing `miniprogram-development` skill for project structure, DevTools, preview, and release rules.
3. Read the existing `wechat-miniprogram-native` skill for native JavaScript, WXML, WXSS, and `setData` conventions.
4. Read only the CloudBase references needed for the current task:
   - architecture and data design: `references/architecture-and-data-contract.md`
   - deployment, MCP/CLI, authentication, and secrets: `references/deployment-security.md`
   - official Tencent skill routing: `references/official-skill-map.md`
   - final validation: `references/review-checklist.md`

Do not copy all official CloudBase skills into context. Route to the smallest relevant subset.

## Architecture contract

Preserve both paths:

```text
Local path:
SS928 -> BLE -> WeChat Mini Program

Remote path:
SS928 -> HTTPS -> CloudBase HTTP Function -> CloudBase database
                                         -> WeChat Mini Program
```

Do not describe these paths as alternatives or recommend removing either one.

Use these capability boundaries:

- Mini Program client:
  - initialize CloudBase once with `wx.cloud.init`
  - use `wx.cloud.callFunction()` for privileged or identity-aware operations
  - use `wx.cloud.database()` only for client-safe reads/writes allowed by collection rules
  - never use browser `@cloudbase/js-sdk` patterns in Mini Program code
- SS928 Linux device:
  - use normal HTTPS requests
  - never use `wx.cloud`
  - upload through a dedicated HTTP Function or another explicitly approved backend endpoint
  - never expose administrator API keys in source control, Mini Program code, logs, or screenshots
- Cloud Functions:
  - use an Event Function for Mini Program SDK calls and OPENID-aware orchestration
  - use an HTTP Function for an externally reachable device REST endpoint
  - do not mix Event Function `exports.main(event, context)` with HTTP Function server code
- Database:
  - create collections explicitly before first write
  - keep privileged writes in functions
  - paginate track and alarm queries

## Development workflow

### 1. Inspect before editing

Confirm:

- `project.config.json` and `miniprogramRoot`
- current `app.js` initialization
- existing BLE service and page data shapes
- whether a CloudBase environment ID is already configured
- current cloud-function root, if any
- existing collections and permission rules

Do not invent missing environment IDs, AppIDs, function names, collection names, or deployed URLs.

### 2. Plan the smallest coherent change

For a medium or large change, output a short plan covering:

- Mini Program files
- cloud-function files
- database collections and indexes
- SS928 uploader files
- security and deployment changes
- test evidence required

Avoid rewriting BLE pages just to add cloud access. Introduce a data-service boundary and preserve existing behavior.

### 3. Keep BLE and cloud data sources decoupled

Prefer a structure such as:

```text
services/
  ble-data-source.js
  cloud-data-source.js
  device-data-service.js
```

Expose stable domain methods such as:

```text
getLatestStatus(deviceId)
getTrackPoints(deviceId, cursor, limit)
getAlarmHistory(deviceId, cursor, limit)
subscribeRealtimeStatus(deviceId, callback)
```

Pages should consume normalized domain objects rather than raw BLE packets or raw database documents.

Do not silently switch from BLE to cloud when the operation requires a specific channel. Surface channel state and errors explicitly.

### 4. Choose the correct CloudBase runtime

Use an Event Function when:

- the Mini Program calls with `wx.cloud.callFunction`
- caller identity from `cloud.getWXContext()` is required
- the operation performs privileged reads/writes or cross-collection validation

Use an HTTP Function when:

- SS928 must `POST` telemetry over public HTTPS
- a REST route such as `/v1/device/telemetry` is required
- the caller is outside the Mini Program runtime

For Node.js HTTP Functions:

- listen on port `9000`
- include `scf_bootstrap`
- parse URL and request body explicitly
- return explicit JSON status codes
- configure public access/security rules intentionally

Use CloudRun instead only when long-lived connections, MQTT brokers, WebSocket services, arbitrary runtimes, or persistent container behavior are genuinely required.

### 5. Apply user and device authorization separately

Mini Program users and physical devices are different principals.

- User identity:
  - initialize with `wx.cloud.init({ env, traceUser: true })`
  - obtain trusted `OPENID` only inside cloud functions with `cloud.getWXContext()`
  - authorize access through a binding record such as `device_bindings`
- Device identity:
  - assign a unique `deviceId`
  - authenticate uploads with an application-layer credential, preferably timestamp + nonce + HMAC signature
  - reject stale timestamps, reused nonces, malformed bodies, unknown devices, and invalid signatures
  - store device secrets in protected configuration, not in Mini Program code or Git

Do not treat OPENID as device authentication. Do not treat a public function URL as proof that a device is trusted.

### 6. Normalize and validate telemetry

Validate all device input before database writes:

- schema version
- device ID
- server-received timestamp and optional device timestamp
- latitude/longitude ranges
- finite roll/pitch/yaw values
- allowed risk levels and alarm types
- payload size and batch size

Keep a latest-status record separate from append-only history. Use server time for ordering when device clocks may drift.

See `references/architecture-and-data-contract.md` for canonical field names and collection design.

### 7. Manage resources honestly

Code authoring does not require MCP. Resource creation, deployment, permission changes, log inspection, and environment management require an actual management path.

Use this order:

1. CloudBase MCP when available and authenticated.
2. Official `tcb` CLI when MCP is unavailable or the user prefers CLI.
3. WeChat DevTools/CloudBase console instructions when neither can be operated from the current environment.

Never claim that a function, collection, rule, or environment was deployed unless the tool output verifies it.

Before any write operation:

- confirm the canonical full EnvId
- inspect tool or CLI command schemas/help
- preserve existing resources
- ask before destructive actions
- verify the result after deployment

### 8. Test both channels

At minimum verify:

- existing BLE connection and parsing still work
- Mini Program CloudBase initialization succeeds
- Mini Program user identity is obtained in a test Event Function
- SS928 or a local test client can upload a signed telemetry sample
- invalid signature and replay attempts are rejected
- latest status can be read only by an authorized user
- track and alarm pagination works
- offline, timeout, permission, and empty-data states are visible in the UI

Use `references/review-checklist.md` before declaring completion.

## Non-negotiable rules

- Keep the project native WeChat Mini Program JavaScript unless the repository explicitly changes that decision.
- Do not introduce Taro, uni-app, browser CloudBase SDK, or a traditional Express server by default.
- Do not hard-code secrets, administrator API keys, or guessed EnvIds.
- Do not let the device write directly to a client-visible database endpoint without server-side validation.
- Do not assume a collection is auto-created by `db.collection(...).add()`.
- Do not generate a Web-style login flow for a native CloudBase Mini Program.
- Do not bind page UI directly to one transport's raw payload format.
- Do not change cloud resources before reading the change-safety and deployment checks in the official CloudBase guidance.
- Do not publish or upload the Mini Program without explicit user approval.

## Expected outputs

When implementing a feature, deliver all applicable items:

- architecture/change plan
- Mini Program source changes
- cloud-function source and package files
- collection definitions, indexes, and permission rules
- SS928 Python/C upload example
- environment/deployment instructions or verified deployment result
- test commands and observed results
- concise README updates

State clearly which parts were implemented, deployed, simulated, or left for real-device validation.
