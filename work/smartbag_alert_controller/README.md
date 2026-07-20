# SS928 SmartBag Alert Controller

Board-side controller for the two-camera dangerous-obstacle warning flow. It reads
vision JSONL events, drives two TM6605/LRA haptic modules over I2C, and plays MAX98357 audio
clips through the existing `/opt/sample/audio/sample_audio 2` path.

## Board Paths

Integrated board root:

```text
/root/smartbag_alert/
  controller/
  intelligent_bag/models/yolo11s.om
  audio/
  config/smartbag.env
```

`intelligent_bag/models/yolo11s.om` is the deployed OM model source. The old
`/opt/sample/yolov8/yolov8n.om` path is only a factory sample compatibility
path and is not treated as the source model for SmartBag.

Put audio files here:

```text
/root/smartbag_alert/audio/L1/audio_chn0.aac
/root/smartbag_alert/audio/L2/audio_chn0.aac
/root/smartbag_alert/audio/L3/audio_chn0.aac
/root/smartbag_alert/audio/L4/audio_chn0.aac
/root/smartbag_alert/audio/R1/audio_chn0.aac
/root/smartbag_alert/audio/R2/audio_chn0.aac
/root/smartbag_alert/audio/R3/audio_chn0.aac
/root/smartbag_alert/audio/R4/audio_chn0.aac
```

`play_hint.txt` is optional in each audio folder. If present, the controller uses
its `sleep_seconds` and `timeout_seconds`; otherwise it uses 5s and 13s.

## TM6605 / LRA Wiring

Route all three I2C devices through one TCA9548A. Connect each LRA only to its
own TM6605 motor connector; never connect an LRA directly to an SS928 GPIO.

| SS928 40Pin | TCA9548A | Notes |
|---:|---|---|
| Pin1 or Pin17 3.3V | VCC | TCA9548A logic supply. |
| Any GND | GND | Common ground for board, BMI270 and both TM6605 modules. |
| Pin3 `I2C0_SDA` | SDA | Board pinmux: `bspmm 0x102F013c 0x2031`. |
| Pin5 `I2C0_SCL` | SCL | Board pinmux: `bspmm 0x102F0140 0x2031`. |
| TCA9548A channel 0 | BMI270 SDA/SCL | BMI270 remains address `0x68`; VCC=3.3V, GND common, CSB=3.3V, SDO=GND. |
| TCA9548A channel 1 | Left TM6605 SDA/SCL | TM6605 VCC=5V, GND common, signal level is 3.3V-compatible. |
| TCA9548A channel 2 | Right TM6605 SDA/SCL | TM6605 VCC=5V, GND common, signal level is 3.3V-compatible. |

Tie TCA9548A address pins A0/A1/A2 to GND for address `0x70`; keep RESET high.
BMI270 must move from the former direct Pin3/Pin5 wiring to channel 0, otherwise
it becomes invisible whenever the controller selects a TM6605 channel.

The old PWM signals on Pin7, Pin32, Pin35, and Pin37 are no longer used for
vibration. Pin7 remains free for its former PWM/I2S alternate function.

## Two-side light-module PWM outputs

Each light module takes only a small-signal PWM input; power the module from
its own rated supply and connect its ground to SS928 ground.  Do not drive a
lamp load directly from a 40Pin signal pin.

| Side | SS928 40Pin | Signal | Pinmux | Behaviour |
|---|---:|---|---|---|
| Left light | Pin7 | `PWM0_OUT10_0_P` (PWM chip 0, channel 10) | `bspmm 0x102F0110 0x1205` | Level 3: 50% PWM for 1 s; level 4: three 80% PWM pulses. |
| Right light | Pin32 | `PWM0_OUT1_0_P` (PWM chip 0, channel 1) | `bspmm 0x102F01EC 0x1201` | Level 3: 50% PWM for 1 s; level 4: three 80% PWM pulses. |

Level-4 pulses use a 200 ms on / 200 ms off rhythm. Levels 1/2, `AL CLEAR`,
and source timeout turn the corresponding light off immediately.

The TM6605 controls LRA resonance internally, so its intensity is selected by
pre-programmed effects rather than a GPIO PWM duty cycle:

| Risk level | Haptic behaviour |
|---:|---|
| 1 / 2 | No vibration. |
| 3 | Medium built-in alert (effect 15) repeated three times, about 2 s total; this is the 80% class. |
| 4 | Strong built-in alert (effect 14) pulsed three times; this is the 100% class. |

MAX98357 audio keeps only the three needed I2S pins:

```text
Pin12 I2S_BCLK, Pin38 I2S_WS, Pin40 I2S_SD_TX
```

Pin7 is not used as I2S MCLK because MAX98357 does not require MCLK.

## Run

Example after copying the vision module to `/root/vision_obstacle_tracker`:

```sh
cd /root/smartbag_alert/controller
python3 smartbag_alert_controller.py \
  --detector-cwd /root/vision_obstacle_tracker \
  --left-detector "python3 vision_obstacle_tracker.py --camera-index 0 --no-display" \
  --right-detector "python3 vision_obstacle_tracker.py --camera-index 1 --no-display"
```

The controller appends `--side left|right --emit-alert-jsonl` to each detector
command if those options are not already present.

## MR20 Ethernet Radar

Copy `mr20_radar.example.json` to `/root/smartbag_alert/config/mr20_radar.json`.
The provided configuration enables the first right-rear radar only. It receives
`192.168.1.200:2369` traffic on board address `192.168.1.102:2368`; configure
that address only on the radar-direct Ethernet interface, with no gateway or
default-route change. The controller combines active camera and radar events by
side and always drives the highest current level. MR20 records are appended as
JSONL under `/root/smartbag_alert/logs/`.

After connecting the radar cable to the board's currently-down `eth1`, verify the
link before changing its temporary address. On the board, use:

```sh
ip -br link show eth1
ip addr add 192.168.1.102/24 dev eth1
ip -br addr show eth1
```

Do not add a gateway or modify `eth0`; remove the temporary address after a
test with `ip addr del 192.168.1.102/24 dev eth1`.

For the deployed OM package, use `om_alert_bridge.py` when the detector runner
emits JSONL or simple `level=1..4` text:

```sh
python3 om_alert_bridge.py \
  --side left \
  --model /root/smartbag_alert/intelligent_bag/models/yolo11s.om \
  --runner "/path/to/your_detector"
```

The bridge keeps the OM model path explicit and emits `vision_alert` JSONL for
the controller. Raw bbox output from the factory YOLO sample is not converted
to danger levels unless `--sample-detection-alerts` is set for development-only
smoke testing.

## Boot Service

Install and enable:

```sh
cp /root/smartbag_alert/controller/smartbag-alert.service /etc/systemd/system/smartbag-alert.service
systemctl daemon-reload
systemctl enable smartbag-alert.service
systemctl restart smartbag-alert.service
```

Runtime configuration lives in `/root/smartbag_alert/config/smartbag.env`.

For an I2C-device precheck only:

```sh
python3 smartbag_alert_controller.py --preflight-only
```

The controller defaults to TCA9548A `0x70`: BMI270 channel 0, left TM6605
channel 1, and right TM6605 channel 2. The BMI270 program re-selects channel 0
for every I2C transaction, while the haptic program locks and selects its own
channel before each write.

For dry-run integration testing from stdin:

```sh
printf '{"type":"vision_alert","side":"left","level":3,"score":0.75,"track_id":1,"ts":1.0}\nAL R4\nAL CLEAR\n' \
  | python3 smartbag_alert_controller.py --dry-run --stdin-jsonl --no-ble
```

## BLE Debug Commands

The controller advertises as `SS928-SmartBag` with Nordic UART Service. Commands:

```text
AL L1
AL L2
AL L3
AL L4
AL R1
AL R2
AL R3
AL R4
AL CLEAR
```

These commands use the same alert state and haptic/light/audio path as real
vision events; until the right TM6605 is installed, `AL R1` through `AL R4` do
not vibrate.
