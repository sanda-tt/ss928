# SS928 new-board migration (2026-07-20)

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
