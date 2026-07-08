# Handoff - BMI270 Backpack BLE Posture v1

Date: 2026-07-05
Workspace: C:\Users\ASUS\Desktop\ss928
Board: root@192.168.1.168, password ebaina

## Completed

- Board-side BMI270 backpack program extended in `work/linux_bmi270_backpack/bmi270_backpack.py`.
- Added calibration recorder with five capture modes:
  - `straight`: 15 s, writes `straight_XX.csv`
  - `hunch`: 15 s, writes `hunch_XX.csv`
  - `straight_walk`: 30 s, writes `straight_walk_XX.csv`
  - `hunch_walk`: 30 s, writes `hunch_walk_XX.csv`
  - `bend_pickup`: 30 s, writes `bend_pickup_XX.csv`
- CSV output directory on the board: `/root/bmi270_calibration`.
- BLE command interface added:
  - `CS <mode> <seconds>` starts capture; short form for WeChat BLE writes.
  - `CE` stops capture.
  - `C?` reports calibration status.
  - `CM` lists modes.
  - Long forms also exist: `CAL_START`, `CAL_STOP`, `CAL_STATUS`, `CAL_MODES`.
- Telemetry frames now include compact `rec` capture status when recording or after a completed capture.
- Updated board configs:
  - `work/linux_bmi270_backpack/config.ss928_ble.json`
  - `work/linux_bmi270_backpack/config.example.json`
- Updated board startup:
  - `work/linux_bmi270_backpack/start_ss928_ble.sh`
  - `work/linux_bmi270_backpack/bmi270-backpack.service`
- Uploaded board program to `/root/linux_bmi270_backpack`.
- Installed systemd autostart:
  - Service path: `/etc/systemd/system/bmi270-backpack.service`
  - `systemctl is-enabled bmi270-backpack` returns `enabled`.
  - `systemctl is-active bmi270-backpack` returns `active`.
- Live board log confirmed BLE and sensor output:
  - `BLE enabled as BMI270-Backpack`
  - `BLE GATT registered`
  - `BLE advertisement registered`
  - JSON frames streaming with roll/pitch/yaw and accel/gyro.
- WeChat mini program updated under `work/ssminiprogram/miniprogram`:
  - Home page is now calibration capture page.
  - Added realtime monitor page under `pages/monitor`.
  - Fixed Chinese text corruption from the previous run.
  - BLE scan/connect/NUS notify/write logic is implemented on both pages.

## Verified Locally

- `node --check work\ssminiprogram\miniprogram\pages\index\index.js`
- `node --check work\ssminiprogram\miniprogram\pages\monitor\index.js`
- `python -m json.tool work\ssminiprogram\miniprogram\app.json`
- `python -m json.tool work\ssminiprogram\miniprogram\pages\index\index.json`
- `python -m json.tool work\ssminiprogram\miniprogram\pages\monitor\index.json`
- `python -m json.tool work\linux_bmi270_backpack\config.ss928_ble.json`
- `python -m py_compile work\linux_bmi270_backpack\bmi270_backpack.py`
- `python -B -m unittest work.linux_bmi270_backpack.tests.test_calibration_recorder -v`
- `sh -n work\linux_bmi270_backpack\start_ss928_ble.sh`

## Verified On Board

Commands used through `skills/ss928-direct-board-debug/scripts/board_debug.py`:

- `probe` connected successfully to `root@192.168.1.168`.
- Uploaded `work/linux_bmi270_backpack` to `/root/linux_bmi270_backpack`.
- Installed and enabled systemd service.
- Restarted `bmi270-backpack`.
- Checked `/tmp/bmi270_ble.log` and systemd status.

Current board facts from the latest run:

- Python: `Python 3.10.12`
- I2C devices present: `/dev/i2c-0` through `/dev/i2c-7`
- hci0 after startup: `38:7A:CC:E9:DA:43`
- Service main process: `python3 /root/linux_bmi270_backpack/bmi270_backpack.py --config /root/linux_bmi270_backpack/config.ss928_ble.json --backend i2c --i2c-bus 0 --i2c-addr 0x68 --ble`
- Current raw pose seen in log: roll about `166.5`, pitch about `-32.0`; this is probably module mounting orientation, not an I2C failure.

## Remaining Tasks

- Test with a real phone in WeChat DevTools preview. The simulator is not enough for BLE validation.
- From the mini program, connect to `BMI270-Backpack` and capture these files:
  - `straight_01.csv`
  - `hunch_01.csv`
  - `straight_walk_01.csv`
  - `hunch_walk_01.csv`
  - `bend_pickup_01.csv`
- After capture, pull CSV files from `/root/bmi270_calibration` and calculate first posture thresholds.
- Decide orientation correction using the actual backpack mounting direction.
- Add threshold/alarm logic after inspecting the captured CSV data.
- Optional: add a page button or status text showing the exact saved remote CSV filename after each capture.

## Unfinished / Watch Items

- Mini program BLE behavior has not been verified on a real phone yet.
- The board service is enabled and running, but reboot persistence has not been tested with an actual reboot.
- `work/linux_bmi270_backpack/__pycache__/bmi270_backpack.cpython-311.pyc` is an untracked generated file from local compile/test.
- `work/linux_bmi270_backpack/tests/` is new and should be kept.
- `work/ssminiprogram/miniprogram/pages/monitor/` is new and should be kept.
- `apply_patch` failed in this Windows sandbox with `helper_unknown_error`; file writes were done using controlled Python UTF-8 scripts.

## Useful Commands

Check board service:

```powershell
python skills\ss928-direct-board-debug\scripts\board_debug.py run "systemctl status bmi270-backpack --no-pager"
python skills\ss928-direct-board-debug\scripts\board_debug.py log --path /tmp/bmi270_ble.log --lines 120
```

Restart board service:

```powershell
python skills\ss928-direct-board-debug\scripts\board_debug.py run "systemctl restart bmi270-backpack"
```

Fetch calibration files later:

```powershell
python skills\ss928-direct-board-debug\scripts\board_debug.py run "find /root/bmi270_calibration -maxdepth 1 -type f -name '*.csv' -print"
```

Open mini program:

- Use WeChat DevTools.
- Project directory: `C:\Users\ASUS\Desktop\ss928\work\ssminiprogram`.
- Preview to phone, then scan/connect `BMI270-Backpack`.
