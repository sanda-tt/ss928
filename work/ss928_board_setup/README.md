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
- `smartbag-alert.service` and `bmi270-backpack.service` retain their historical
  runtime paths and start after the WS73 controller is visible.

The GNSS tracker retains its verified manual command
`cd /root/dx_gp21_tracker && sh ./start_ss928_gnss_track.sh`; it is not enabled
as a separate boot service because no previous persistent unit was recorded.

## Hardware-ready verification

The migration was completed before the external modules were wired. The two
business services are enabled for future boots but were intentionally left
stopped after configuration so missing I2C/PWM devices do not create restart
loops. After the wiring is complete, use:

```sh
systemctl start smartbag-alert.service
systemctl start bmi270-backpack.service
systemctl status smartbag-alert.service bmi270-backpack.service --no-pager
networkctl status eth1
ip route get 192.168.1.200
```

The GNSS tracker remains manual until its UART and BLE behavior is checked with
the connected module. Do not change `eth1` to `/24` and do not add a gateway.
