# SS928 SmartBag Alert Controller

Board-side controller for the two-camera dangerous-obstacle warning flow. It reads
vision JSONL events, drives four PWM vibration inputs, and plays MAX98357 audio
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

## 40Pin PWM Wiring

The four vibration modules must share GND with the SS928 board. The modules have
their own power supply; connect only the PWM control inputs to these 40Pin pins.

| Side | Motor | 40Pin | PWM signal | pwmchip/channel | Pinmux |
|---|---|---:|---|---|---|
| Left | 1 | Pin7 | PWM0_OUT10_0_P | pwmchip0/pwm10 | `bspmm 0x102F0110 0x1205` |
| Left | 2 | Pin32 | PWM0_OUT1_0_P | pwmchip0/pwm1 | `bspmm 0x102F01EC 0x1201` |
| Right | 1 | Pin35 | PWM0_OUT14_0_P | pwmchip0/pwm14 | `bspmm 0x102F0100 0x1205` |
| Right | 2 | Pin37 | PWM0_OUT15_0_P | pwmchip0/pwm15 | `bspmm 0x102F00DC 0x1205` |

PWM period is `1000000 ns`; 60% is `600000`, 100% is `1000000`.

MAX98357 audio keeps only the three needed I2S pins:

```text
Pin12 I2S_BCLK, Pin38 I2S_WS, Pin40 I2S_SD_TX
```

Pin7 is not used as I2S MCLK because MAX98357 does not require MCLK and Pin7 is
reserved here for the left-1 vibration PWM signal.

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

For a hardware precheck only:

```sh
python3 smartbag_alert_controller.py --preflight-only
```

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

These commands use the same alert state and PWM/audio path as real vision events.
