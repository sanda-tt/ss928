### work/imu_fall_detector

- Goal: pure Python IMU fall/impact detector for SS928 Linux integration. Input is timestamp plus ax/ay/az in g and gx/gy/gz in deg/s.
- Algorithm: threshold + finite state machine with NORMAL, POSSIBLE_FALL, IMPACT, POSTURE_CHANGED, FALL_CONFIRMED, and IMPACT_ONLY. It uses a sliding-window filter, resultant acceleration, resultant gyro, vector jerk, stationary detection, and posture-angle change from the gravity vector.
- Key files: `work/imu_fall_detector/imu_fall_detector.py` is the importable module; `run_detector.py` reads CSV or JSONL stdin and prints compact JSON events; `config.example.json` contains tunable 50 Hz thresholds; `tests/` uses stdlib unittest.
- Integration note: construct `ImuSample(t, ax, ay, az, gx, gy, gz)` in the main program and call `FallImpactDetector.update(sample)`. Normal frames return `[]`; transition frames return JSON-serializable event dicts. Use `event_to_json(event)` for JSONL output.
- Verification on 2026-07-05: `python -B -m unittest discover -s work/imu_fall_detector/tests` passed 6 tests; `python -B work/imu_fall_detector/run_detector.py --simulate` emitted POSSIBLE_FALL -> IMPACT -> POSTURE_CHANGED -> FALL_CONFIRMED JSON events.
- Tuning caution: defaults are starting thresholds only. Re-tune with real mounting orientation, normal impacts, and real fall-like data before relying on alert behavior.
