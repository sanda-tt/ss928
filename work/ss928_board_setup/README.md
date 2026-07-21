# SS928 board setup and migration layout

This directory contains the persistent system configuration used to rebuild the
SS928 board without modifying the vendor samples under `/opt/sample`.

## Board workspace tree

```text
/root/
|-- work/                         # board-side project source, mirrors local work/
|   |-- smartbag_alert_controller/
|   |-- linux_bmi270_backpack/
|   |-- dx_gp21_tracker/
|   |-- bmi270_i2c_pose/
|   |-- imu_fall_detector/
|   |-- imx347_mipi_preview/
|   |-- max98357_i2s_test/
|   |-- mt5710_voice_call/
|   |-- mt5710_5g_cloud_upload/
|   |-- smartbag_cloud_uploader/
|   `-- radar/
|-- config/
|   `-- smartbag_alert/
|       |-- smartbag.env
|       `-- mr20_radar.json
|-- data/
|   |-- bmi270_calibration/
|   |-- dx_gp21_tracks/
|   `-- smartbag_alert/{logs,runtime}/
`-- smartbag_alert/
    |-- controller -> /root/work/smartbag_alert_controller
    |-- audio -> /root/work/smartbag_alert_audio_deploy/audio
    |-- config -> /root/config/smartbag_alert
    |-- logs -> /root/data/smartbag_alert/logs
    |-- runtime -> /root/data/smartbag_alert/runtime
    `-- intelligent_bag/models/yolo11s.om
```

Compatibility links retain the previously verified paths
`/root/linux_bmi270_backpack`, `/root/dx_gp21_tracker`,
`/root/dx_gp21_tracks`, `/root/bmi270_calibration`, and
`/root/max98357_i2s_test`.

## Persistent services and network

- `20-mr20-radar.network` configures only `eth1` as `192.168.1.102/32`
  and adds the host route to `192.168.1.200/32`. It never changes `eth0`,
  adds no gateway, and adds no default route.
- `ws73-bluetooth-module.service` loads the vendor WS73 kernel modules before
  the system `bluetooth.service`. It deliberately does not use the private
  DBus instance created by the vendor `ble.sh` helper.
- `smartbag-5g-upload.service` is the single telemetry coordinator. It waits for
  MT5710 with bounded timeouts, then starts the enabled telemetry producers and
  uploads their current state. DX-GP21-A is currently unplugged/fault-isolated,
  so the installed unit uses `--skip-gnss` and starts BMI270 only. The unit is
  `Type=simple`, retries after five seconds, and has a ten-second stop timeout,
  so it does not block the rest of boot.
- `smartbag-alert.service` independently owns the one-sided MR20 warning path,
  PWM lights, TCA9548A/TM6605 haptics, audio, and the `SS928-SmartBag` BLE
  advertisement.
- `bmi270-backpack.service` is disabled to prevent a second process from
  opening the same BMI270. BMI270 is launched by `smartbag-5g-upload.service`
  and reuses the configured system BlueZ controller without starting the
  vendor private D-Bus/bluetoothd stack.
- The DX-GP21 tracker is normally launched by `smartbag-5g-upload.service`; no
  separate GNSS boot unit is needed. It is intentionally disabled by the
  current `--skip-gnss` service option until the unplugged module is repaired.

### Do not revive superseded BMI/BlueZ entry points

On this integrated SmartBag board, `smartbag-5g-upload.service` is the **only**
owner allowed to start `bmi270_backpack.py`. The following old or standalone
entry points remain in the tree only for reference/compatibility and must not
be started alongside it:

- Do not run `systemctl start bmi270-backpack.service` or enable that unit. The
  file may remain installed, but its required state is `disabled` and
  `inactive`.
- Do not manually launch `bmi270_backpack.py --ble` or
  `start_ss928_ble.sh` while `smartbag-5g-upload.service` is active. The same
  script is already invoked as a child of the unified service.
- Do not run `/opt/sample/ws73/ble.sh 0` and do not set
  `START_BLE_STACK=1` on this board. They start a second vendor D-Bus/
  `bluetoothd` stack and conflict with the system BlueZ instance.

The expected steady state is one BMI270 process in the
`smartbag-5g-upload.service` cgroup and one system
`/usr/lib/bluetooth/bluetoothd` process. If the old unit is found active, use:

```sh
systemctl disable --now bmi270-backpack.service
systemctl restart smartbag-5g-upload.service
```

Do not delete the legacy unit or compatibility paths: keeping them disabled is
intentional, and the source directory is still used by the unified service.

## Hardware-ready verification

With the external modules connected, the persistent runtime is checked with:

```sh
systemctl status smartbag-5g-upload.service smartbag-alert.service --no-pager
systemctl is-enabled smartbag-5g-upload.service smartbag-alert.service \
  ws73-bluetooth-module.service bluetooth.service bmi270-backpack.service
systemctl is-active bmi270-backpack.service
pgrep -a -f 'bmi270_backpack.py|bluetoothd'
networkctl status eth1
ip route get 192.168.1.200
```

DX-GP21-A is currently unplugged after a module fault. It is not a boot, 5G,
BMI270, fall-alarm, or CloudBase prerequisite: do not run its tracker or spend
time diagnosing UART4 until the module is repaired and deliberately restored.
Do not change `eth1` to `/24` and do not add a gateway.
