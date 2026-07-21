# SS928 new-board migration (2026-07-20)

> **Supersession notice (2026-07-21):** The earlier migration-stage statements
> below saying `bmi270-backpack.service` was enabled are historical only. The
> final integrated system supersedes them: that unit must remain
> `disabled`/`inactive`, and BMI270 is owned only by
> `smartbag-5g-upload.service`.

## Goal

Rebuild a clean SS928 Ubuntu 22.04 board at `192.168.1.168` from the verified
local project copies and prior board records, organize the board workspace,
restore persistent network settings, and restore boot service definitions.

## Clean-board baseline

- Ubuntu 22.04.1 LTS, kernel 4.19.90, Python 3.10.12.
- `eth0=192.168.1.168/24` was already working and remained untouched.
- `eth1` had no address or network file.
- `/root` contained no prior project tree.
- The WS73 vendor files existed under `/opt/sample/ws73`, but `ble_soc` was not
  loaded and the system BlueZ service had no `hci0` controller.

## Restored board layout

- Board project source is mirrored under `/root/work/`:
  `smartbag_alert_controller`, `linux_bmi270_backpack`, `dx_gp21_tracker`,
  `bmi270_i2c_pose`, `imu_fall_detector`, `imx347_mipi_preview`,
  `max98357_i2s_test`, `mt5710_voice_call`, `radar`, and
  `ss928_board_setup`.
- Persistent data is under `/root/data/`; SmartBag configuration is under
  `/root/config/smartbag_alert/`.
- Compatibility symlinks preserve the old `/root/smartbag_alert`,
  `/root/linux_bmi270_backpack`, `/root/dx_gp21_tracker`,
  `/root/dx_gp21_tracks`, `/root/bmi270_calibration`, and other project paths.
- The eight `L1..L4` and `R1..R4` AAC files were restored under
  `/root/smartbag_alert/audio/<id>/audio_chn0.aac`.
- `yolo11s.om` was restored to
  `/root/smartbag_alert/intelligent_bag/models/yolo11s.om`; its board SHA-256
  matched the local source: `19415b173c90122089f0a63d1cb54d9eba2904905438a980cb4e44f17c8981c0`.

## Persistent system settings

- `/etc/systemd/network/20-mr20-radar.network` gives only `eth1` the address
  `192.168.1.102/32` and a link route to `192.168.1.200/32`. It has no gateway
  and does not alter the `eth0` management route.
- `ws73-bluetooth-module.service` loads the vendor WS73 modules before the
  normal system `bluetooth.service`; it does not start the vendor private DBus.
- `smartbag-alert.service` and `bmi270-backpack.service` are installed and
  enabled. Both were intentionally left stopped until the user connects the
  external hardware.
- The GNSS tracker remains a manual service, matching the previous recorded
  workflow: `cd /root/dx_gp21_tracker && sh ./start_ss928_gnss_track.sh`.

## Verification

- Local and board tests passed: SmartBag 22, BMI270 6, DX-GP21 6.
- All four board shell scripts passed `sh -n` after LF normalization.
- `systemd-analyze verify` passed for the three installed unit files.
- `systemd-networkd`, `ws73-bluetooth-module`, and `bluetooth` are enabled and
  active; `hci0` is visible through `btmgmt info`.
- `smartbag-alert` and `bmi270-backpack` are enabled and inactive by design.
- `eth1` is configured with the expected `/32` address and route. It showed
  `no-carrier` because the MR20 cable was not connected.

## Hardware follow-up

No external hardware was wired during migration. An expected BMI270 probe miss
and expected SmartBag PWM/TCA initialization errors were observed before that
fact was clarified; they are not treated as code failures. After wiring, start
the two enabled services and validate I2C0/TCA9548A/BMI270, PWM lights, TM6605,
MR20 UDP, BLE advertisements, UART4 GNSS, and audio on the real hardware.

## Full-system autostart completion (2026-07-21)

The connected board now uses two independent `Type=simple` services instead of
adding another controller:

- `smartbag-5g-upload.service` owns MT5710 NCM setup, DX-GP21 tracking,
  BMI270 posture/fall processing, and CloudBase upload. It is enabled with
  five-second restart delay and ten-second stop timeout.
- `smartbag-alert.service` owns the temporarily one-sided right MR20 input,
  four-level warning decisions, audio, PWM lights, both muxed TM6605 outputs,
  and its BLE advertisement.
- `ws73-bluetooth-module.service` and the normal `bluetooth.service` are
  enabled. The old `bmi270-backpack.service` is disabled so it cannot contend
  for the sensor.

Two boot-time defects were fixed before acceptance. PWM setup now skips a
redundant `enable=0` write when a freshly exported channel already reads zero;
this avoids the board driver's `EINVAL` while its period is still zero. The
BMI270 startup path now honors `START_BLE_STACK=0` immediately and reuses the
system BlueZ instance, avoiding the previous 30-second controller wait and a
second vendor bluetoothd/D-Bus conflict.

The final cold-path validation used a full reboot. MT5710 was ready two seconds
after the telemetry unit started; DX-GP21 and BMI270 were launched immediately;
BMI270 GATT and advertising registered about five seconds later. Both main
services, WS73, and BlueZ were active, while the old BMI service remained
disabled/inactive. The right MR20 listener was bound to
`192.168.1.102:2368`, its `/32` host route was present, and real frames from
`192.168.1.200` continued appending to the JSONL log. Both PWM outputs recovered
to `enable=0`, `period=1000000`, `duty_cycle=0`, so the left lamp stayed off at
idle. Earlier in the same connected-hardware run, a real right-radar target was
classified L2 (4.0 m, -2.0 m/s, TTC 2.0 s).

CloudBase readback after the final reboot showed a new live BMI attitude and
posture timestamp in `device_status`, plus the matching fresh timestamp in
`posture_daily_stats`. This proves the board-to-MT5710-to-cloud path. Existing
location/risk fields in `device_status` are retained by partial updates and are
not evidence of a current GNSS fix.

GNSS is the only unresolved hardware gate. The service and UART4 tracker are
running, but `/tmp/smartbag_latest_location.json` is absent and a controlled
direct read returned zero bytes at 9600, 38400, 57600, and 115200 baud. Check
DX-GP21 power/enable, common ground, and module TX to Pin10/UART4_RXD. The
tracker remains supervised and will begin parsing/uploading once bytes appear.

Local regression totals after these changes: SmartBag alert 24,
MT5710/autostart 14, cloud uploader 5, BMI270 15, and DX-GP21 8 tests, all
passing. Board `sh -n` also passed for the three deployed startup scripts.

## Abandoned entry-point guard (2026-07-21)

During later GNSS diagnostics, the disabled legacy
`bmi270-backpack.service` was found manually active. It created a second
BMI270 reader and attempted to start another vendor `bluetoothd`, producing an
`Address already in use`/GATT database conflict. Stopping that legacy unit
restored the intended single-process state.

Future board work must preserve these rules:

- Never start or enable `bmi270-backpack.service` on the integrated board.
- Never manually run `start_ss928_ble.sh` or `bmi270_backpack.py --ble` while
  `smartbag-5g-upload.service` is active. The unified service already starts
  the script with `START_BLE_STACK=0`.
- Never run `/opt/sample/ws73/ble.sh 0` or force `START_BLE_STACK=1` on this
  board; system `bluetooth.service` is the only BlueZ owner.
- Do not delete the legacy unit/source paths. They are retained for history and
  standalone testing, but their processes are abandoned in the integrated
  runtime.
- Acceptance state: `smartbag-5g-upload.service=active`,
  `bmi270-backpack.service=disabled/inactive`, exactly one
  `bmi270_backpack.py`, and exactly one system `bluetoothd`.
