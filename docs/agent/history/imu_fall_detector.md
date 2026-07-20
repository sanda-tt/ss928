### work/imu_fall_detector

- Goal: pure Python IMU fall/impact detector for SS928 Linux integration. Input is timestamp plus ax/ay/az in g and gx/gy/gz in deg/s.
- Algorithm: threshold + finite state machine with NORMAL, POSSIBLE_FALL, IMPACT, POSTURE_CHANGED, FALL_CONFIRMED, and IMPACT_ONLY. It uses a sliding-window filter, resultant acceleration, resultant gyro, vector jerk, stationary detection, and posture-angle change from the gravity vector.
- Key files: `work/imu_fall_detector/imu_fall_detector.py` is the importable module; `run_detector.py` reads CSV or JSONL stdin and prints compact JSON events; `config.example.json` contains tunable 50 Hz thresholds; `tests/` uses stdlib unittest.
- Integration note: construct `ImuSample(t, ax, ay, az, gx, gy, gz)` in the main program and call `FallImpactDetector.update(sample)`. Normal frames return `[]`; transition frames return JSON-serializable event dicts. Use `event_to_json(event)` for JSONL output.
- Verification on 2026-07-05: `python -B -m unittest discover -s work/imu_fall_detector/tests` passed 6 tests; `python -B work/imu_fall_detector/run_detector.py --simulate` emitted POSSIBLE_FALL -> IMPACT -> POSTURE_CHANGED -> FALL_CONFIRMED JSON events.
- Tuning caution: defaults are starting thresholds only. Re-tune with real mounting orientation, normal impacts, and real fall-like data before relying on alert behavior.

#### 2026-07-20 SS928 competition fusion MVP

- Added `fall_fusion.py` and `config.ss928.json` for the three-condition gate: IMU `FALL_CONFIRMED`, a camera/radar level-3 or level-4 warning in the preceding 10 seconds, and no sustained return to standing posture during the following 7 seconds.
- `work/smartbag_alert_controller` writes the latest qualifying warning to `/tmp/smartbag_last_high_warning.json`; manual commands and levels 1/2 do not count.
- `work/linux_bmi270_backpack/fall_fusion_runtime.py` converts board SI units to g and deg/s, runs the detector in the existing 50 Hz loop, and appends `fall_alarm` JSONL to `/root/data/smartbag_alert/logs/fall_alarm.jsonl`.
- Local verification passed 11 IMU/fusion tests, 9 BMI270 tests, and 23 SmartBag controller tests. No board upload or physical IMU/radar/camera validation was performed because the peripherals are not connected.

#### 2026-07-20 board deployment

- Uploaded the synchronized projects to `/root/work/imu_fall_detector`, `/root/work/linux_bmi270_backpack`, and `/root/work/smartbag_alert_controller`; seven key source/config SHA-256 values matched the Windows workspace.
- Board Python 3.10 passed 11 IMU/fusion tests, 9 BMI270 tests, and 23 SmartBag controller tests. Both startup scripts passed `sh -n`, both JSON configs parsed, and both systemd units passed `systemd-analyze verify`.
- Installed the repository service files under `/etc/systemd/system`, ran `daemon-reload`, enabled both services, and restarted them. Both `multi-user.target.wants` links exist, so the programs are configured for boot startup.
- Physical runtime is intentionally unverified: with peripherals disconnected, `bmi270-backpack.service` auto-restarts on TCA9548A I2C `EIO`; `smartbag-alert.service` auto-restarts on PWM `EINVAL` and TM6605 mux `EIO`. Re-check stable service state and a real fused alarm after wiring the mux, BMI270, output modules, radar, and cameras.
