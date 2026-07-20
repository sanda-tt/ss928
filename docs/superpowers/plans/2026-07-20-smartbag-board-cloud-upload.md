# SmartBag Board Cloud Upload Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Connect DX-GP21-A tracks, processed BMI270 posture, daily posture totals, and final fall alarms to the existing Tencent WeChat CloudBase telemetry and Mini Program read paths.

**Architecture:** Existing board processes call one small standard-library Python uploader. The existing `smartbag-device-ingest` HTTP Function merges partial latest status and writes append-only tracks/alarms plus absolute daily posture totals; the existing `smartbag-app-api` Event Function remains the Mini Program read boundary.

**Tech Stack:** Python 3.10 standard library, Node.js 18 CommonJS, Tencent CloudBase HTTP/Event Functions, CloudBase NoSQL, native WeChat Mini Program JavaScript, `unittest`, Node `assert` tests.

## Global Constraints

- The demo device ID remains exactly `bag001`.
- The upload route and `X-Upload-Token` protocol remain unchanged; secrets stay outside Git and logs.
- DX-GP21-A is the only GNSS data source; MT5710 provides network transport only.
- Camera/MR20 warnings are not uploaded; only the existing final `FALL_ALARM` event is uploaded.
- Existing BLE, local JSONL, light, vibration, audio, and detection behavior must remain intact.
- Do not connect hardware, change board networking, or start/restart board services in this phase.
- Production code follows a witnessed RED -> GREEN test cycle.

---

### Task 1: Shared board telemetry client

**Files:**
- Create: `work/smartbag_cloud_uploader/__init__.py`
- Create: `work/smartbag_cloud_uploader/telemetry_client.py`
- Create: `work/smartbag_cloud_uploader/tests/test_telemetry_client.py`

**Interfaces:**
- Produces: `TelemetryClient`, `build_track_payload(point)`, `build_posture_payload(snapshot, daily)`, `build_fall_payload(event, location)`, `read_fresh_location(path, now_ms, max_age_ms)`.
- `TelemetryClient.submit_latest_status(payload)` replaces a pending status snapshot; `submit_event(payload)` enqueues append-only track/alarm work; `close()` stops its worker.

- [ ] **Step 1: Write failing payload tests**

Add `unittest` cases asserting that track points contain unique per-point IDs, posture contains `good/bad` plus calibrated angles but no raw `a`/`w` arrays, fall payloads require `FALL_ALARM`, and stale location caches are omitted.

- [ ] **Step 2: Run tests and verify RED**

Run: `python -B -m unittest discover -s work/smartbag_cloud_uploader/tests -v`

Expected: import failure because `telemetry_client.py` does not exist.

- [ ] **Step 3: Implement payload builders and bounded background client**

Use `urllib.request.Request`, a five-second timeout, `queue.Queue(maxsize=...)`, a one-slot latest-status buffer, daemon worker, environment-only token loading, and redacted error messages. Build top-level telemetry objects with `version`, `deviceId`, `requestId`, `reportedAt`, `status`, `trackPoints`, `alarms`, and optional `postureDaily`.

- [ ] **Step 4: Run tests and verify GREEN**

Run: `python -B -m unittest discover -s work/smartbag_cloud_uploader/tests -v`

Expected: all shared uploader tests pass.

- [ ] **Step 5: Commit Task 1 files**

Run: `git add work/smartbag_cloud_uploader && git commit -m "feat: add smartbag telemetry client"`

### Task 2: DX-GP21-A track upload integration

**Files:**
- Modify: `work/dx_gp21_tracker/dx_gp21_tracker.py`
- Modify: `work/dx_gp21_tracker/config.ss928_uart4.json`
- Modify: `work/dx_gp21_tracker/README.md`
- Modify: `work/dx_gp21_tracker/tests/test_dx_gp21_tracker.py`

**Interfaces:**
- Consumes: `TelemetryClient`, `build_track_payload(point)`.
- Produces: `TrackerApp` injection point `telemetry_client=None`; uploads only after `TrackStore.append(point)` succeeds and atomically writes the latest-location cache.

- [ ] **Step 1: Write failing GNSS integration tests**

Add a fake telemetry client and assert one valid NMEA point is locally stored, submitted once, and cached; assert invalid fixes submit nothing and original BLE frame behavior is unchanged.

- [ ] **Step 2: Run tests and verify RED**

Run: `python -B -m unittest discover -s work/dx_gp21_tracker/tests -v`

Expected: failure because `TrackerApp` has no telemetry injection or upload configuration.

- [ ] **Step 3: Implement minimal integration**

Extend config with `cloud_upload.enabled`, `cloud_upload.latest_location_path`, queue and timeout values. Add the repository `work` parent to `sys.path`, construct the client only when enabled, submit after successful local append, and close it during shutdown. Keep `--simulate` usable without a token.

- [ ] **Step 4: Run GNSS and syntax tests**

Run: `python -B -m unittest discover -s work/dx_gp21_tracker/tests -v`

Run: `python -B work/dx_gp21_tracker/dx_gp21_tracker.py --simulate --once --no-ble --track-dir work/dx_gp21_tracker/tmp_test_tracks`

Expected: tests pass and simulation exits zero without hardware.

- [ ] **Step 5: Commit Task 2 files**

Run: `git add work/dx_gp21_tracker && git commit -m "feat: upload dx gp21 cloud tracks"`

### Task 3: Processed BMI270 posture, daily totals, and final fall upload

**Files:**
- Modify: `work/linux_bmi270_backpack/bmi270_backpack.py`
- Modify: `work/linux_bmi270_backpack/fall_fusion_runtime.py`
- Modify: `work/linux_bmi270_backpack/config.example.json`
- Modify: `work/linux_bmi270_backpack/config.ss928_ble.json`
- Modify: `work/linux_bmi270_backpack/README.md`
- Create: `work/linux_bmi270_backpack/posture_cloud.py`
- Create: `work/linux_bmi270_backpack/tests/test_posture_cloud.py`
- Modify: `work/linux_bmi270_backpack/tests/test_fall_fusion_runtime.py`
- Modify: `work/linux_bmi270_backpack/tests/test_calibration_recorder.py`

**Interfaces:**
- Consumes: shared payload builders/client and DX-GP21-A latest-location cache.
- Produces: `AnomalyDetector.is_condition_active("HUNCH")`, `PostureDailyAccumulator.update(status, now_wall, sample_valid=True)`, `PostureCloudReporter.tick(state, now_mono, now_wall)`, and an additional final-fall sink.

- [ ] **Step 1: Write failing posture-state tests**

Assert HUNCH stays `good` before the existing hold time, becomes `bad` only after the existing hold/motion gates, and clears to `good` when the condition clears. Assert payload angles are calibrated and raw IMU arrays are absent.

- [ ] **Step 2: Run focused tests and verify RED**

Run: `python -B -m unittest work/linux_bmi270_backpack/tests/test_calibration_recorder.py work/linux_bmi270_backpack/tests/test_posture_cloud.py -v`

Expected: missing `is_condition_active` and `posture_cloud` failures.

- [ ] **Step 3: Implement readable condition state and reporter**

Record each condition's post-hold active state inside `AnomalyDetector` without changing thresholds, cooldowns, or emitted alerts. Build `PostureCloudReporter` to upload at a configurable five-second interval with `posture_status`, `is_wearing`, `reminder_active`, and calibrated `attitude`.

- [ ] **Step 4: Write failing daily accumulator tests**

Assert absolute wearing/good/bad seconds, date rollover, atomic JSON persistence, reload after restart, clock-gap clamping, and `good + bad <= wearing`.

- [ ] **Step 5: Implement daily accumulator and verify GREEN**

Use a local JSON file, `Path.replace()` atomic writes, a maximum accounted gap, and absolute `postureDaily` snapshots.

Run: `python -B -m unittest work/linux_bmi270_backpack/tests/test_posture_cloud.py -v`

Expected: all posture cloud tests pass.

- [ ] **Step 6: Write failing final-fall upload test**

Inject separate local-log and cloud sinks, feed the existing real fall sequence, and assert exactly one cloud alarm is built only from the returned `FALL_ALARM`. Assert recent cached location is included and stale location is omitted.

- [ ] **Step 7: Implement additive final-fall sink**

Preserve `_emit_alarm` local JSONL behavior, then call the injected cloud sink after final alarm creation. Do not upload warning markers, IMPACT, FREEFALL, or pending states.

- [ ] **Step 8: Run full BMI/fall suite**

Run: `python -B -m unittest discover -s work/linux_bmi270_backpack/tests -v`

Run: `python -B -m unittest discover -s work/imu_fall_detector/tests -v`

Expected: all existing and new tests pass.

- [ ] **Step 9: Commit Task 3 files**

Run: `git add work/linux_bmi270_backpack && git commit -m "feat: upload processed posture and falls"`

### Task 4: Extend the CloudBase telemetry HTTP Function

**Files:**
- Modify: `work/ssminiprogram/cloudfunctions/smartbag-device-ingest/lib/telemetry-core.js`
- Modify: `work/ssminiprogram/cloudfunctions/smartbag-device-ingest/index.js`
- Modify: `work/ssminiprogram/tests/smartbag-cloud.test.js`

**Interfaces:**
- Consumes: telemetry v1 optional `status`, `trackPoints`, `alarms`, and `postureDaily`.
- Produces: `store.mergeStatus(document)`, `store.setPostureDaily(document)`, validation errors with safe status codes, and existing track/alarm methods.

- [ ] **Step 1: Write failing CloudBase processor tests**

Add tests for partial status merge, preservation of unrelated location/posture fields, postureDaily stable document shape, invalid coordinates/numbers/status/totals, missing per-record request IDs, and old valid payload compatibility.

- [ ] **Step 2: Run Node test and verify RED**

Run from `work/ssminiprogram`: `node tests/smartbag-cloud.test.js`

Expected: failures because `mergeStatus`, `postureDaily`, and strict validation are absent.

- [ ] **Step 3: Implement processor validation and writes**

Validate before any database write. Replace full status set with top-level partial merge, append one point/alarm at a time, and write daily absolute totals to `_id = bag001_<date>` with `_id` removed from the set body.

- [ ] **Step 4: Implement CloudBase Node SDK store operations**

Use `doc(id).update(partial)` for existing status and fall back to `doc(id).set(partial)` only for a missing document. Use direct Node SDK document arguments, never `{ data: document }`. Use `doc(id).set(dataWithoutId)` for daily totals.

- [ ] **Step 5: Run Node tests and syntax checks**

Run from `work/ssminiprogram`:

`node tests/smartbag-cloud.test.js`

`node --check cloudfunctions/smartbag-device-ingest/index.js`

`node --check cloudfunctions/smartbag-device-ingest/lib/telemetry-core.js`

Expected: tests and syntax checks pass.

- [ ] **Step 6: Commit Task 4 files**

Run: `git add work/ssminiprogram/cloudfunctions/smartbag-device-ingest work/ssminiprogram/tests/smartbag-cloud.test.js && git commit -m "feat: extend cloudbase telemetry ingest"`

### Task 5: Local cross-module simulation and Mini Program contract verification

**Files:**
- Modify: `work/ssminiprogram/tools/simulate_device_upload.py`
- Create: `work/ssminiprogram/tests/board-upload-contract.test.js`
- Modify only if a failing test requires it: `work/ssminiprogram/miniprogram/utils/posture-utils.js`
- Modify only if a failing test requires it: `work/ssminiprogram/miniprogram/services/cloud-data-source.js`

**Interfaces:**
- Produces: repeatable `--dry-run` and selected sample modes for track, posture, daily posture, and final fall.

- [ ] **Step 1: Write failing contract test**

Assert `getRealtimePosture` accepts the board's `posture_status/is_wearing/reminder_active`, daily mapping accepts absolute totals, track mapping accepts DX-GP21-A location fields, and alarm mapping accepts `fall_detected` while ignoring ordinary warnings.

- [ ] **Step 2: Run contract tests and verify RED where mapping is missing**

Run from `work/ssminiprogram`: `node tests/board-upload-contract.test.js`

Expected: either a specific missing mapping failure or, if the current mapping already satisfies the contract, record that no production Mini Program change is necessary and keep the new regression test.

- [ ] **Step 3: Extend simulator and make the minimum mapping correction**

Add deterministic dry-run payload generation for the four data classes. Do not read or print credentials during dry-run. Change client mapping only for a witnessed failure.

- [ ] **Step 4: Run all Mini Program source tests**

Run from `work/ssminiprogram`:

`node tests/smartbag-cloud.test.js`

`node tests/track-utils.test.js`

`node tests/posture-display.test.js`

`node tests/posture-analysis-page.test.js`

`node tests/alarm-history-page.test.js`

`node tests/board-upload-contract.test.js`

Expected: all tests pass.

- [ ] **Step 5: Commit Task 5 files**

Run: `git add work/ssminiprogram/tools/simulate_device_upload.py work/ssminiprogram/tests work/ssminiprogram/miniprogram && git commit -m "test: cover board cloud upload contract"`

### Task 6: CloudBase deployment, readback, and documentation

**Files:**
- Modify: `work/ssminiprogram/README.md`
- Create: `docs/agent/history/smartbag-board-cloud-upload.md`
- Modify: `docs/agent/history/README.md`

**Interfaces:**
- Uses CloudBase MCP for the bound environment only.
- Deploys only `smartbag-device-ingest` unless source evidence requires `smartbag-app-api` changes.

- [ ] **Step 1: Run the complete local verification suite**

Run all Python tests for shared uploader, DX-GP21-A, BMI270, and fall fusion; run all relevant Node tests and `git diff --check`.

- [ ] **Step 2: Re-audit CloudBase before write**

Confirm auth/environment, function type/runtime, gateway access, collection existence/indexes/permissions, and preserve the current environment variables without exposing them.

- [ ] **Step 3: Deploy only changed function code**

Use `manageFunctions(action="updateFunctionCode", functionName="smartbag-device-ingest", functionRootPath="C:\\Users\\ASUS\\Desktop\\ss928\\work\\ssminiprogram\\cloudfunctions")` and wait for Active/Available status.

- [ ] **Step 4: Send controlled simulated uploads**

Send one track, one posture/daily snapshot, and one final fall payload through the existing HTTPS endpoint. Require HTTP 200 for each and never print the token.

- [ ] **Step 5: Verify NoSQL records and Event Function reads**

Use MCP to confirm `device_status`, `track_points`, `posture_daily_stats`, and `alarm_history`. Invoke `smartbag-app-api` actions `getRealtimePosture`, `getDailyPosture`, `getTrackPoints`, and `getAlarmHistory` and confirm the uploaded request IDs/status appear.

- [ ] **Step 6: Update documentation with honest boundaries**

Record implemented files, tests, CloudBase deployment/readback evidence, and explicitly list unverified DX-GP21-A, BMI270, MT5710, physical fall, and WeChat real-device checks.

- [ ] **Step 7: Final verification and commit**

Run the complete suite again, inspect `git diff --check`, then commit only task-owned files with `git commit -m "docs: record smartbag cloud upload integration"`.
