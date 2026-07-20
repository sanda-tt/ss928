# SS928 factory YOLOv8 + VOT

This is the independent WSL2-cross-compiled project for the factory
`/opt/sample/yolov8/yolov8n.om`.  It does not modify or reconvert the OM and it
does not use OpenMP or `libgomp`.

## Current acceptance status (2026-07-20)

- Passed: SDK/toolchain inventory; live OM/AIPP inspection; exact reuse of
  `vot.c` and `vot_backend.cpp`; preprocess/decode/contract/pipeline tests;
  AArch64 cross-build; one-frame and 100-frame ACL execution; allocation-free
  steady-state inference; clean shutdown; dependency check.
- Passed with synthetic detections: stable tracking, copied VOT distance,
  relative speed, TTC/risk and clean JSONL generation.
- Failed stage G: the unchanged factory OM returned no target candidate at
  `--conf 0.25` for the repository double-decker-bus image.  A known COCO bus
  diagnostic image also produced abnormally low target-class maxima.  ACL
  execution itself returned success and the output contained no NaN/Inf.
- Not entered: USB capture, 10-minute camera run, physical distance
  calibration and live controller integration.  The strict stage gate forbids
  starting these after the real-detection failure.
- Existing controller caveat: its stock `--dry-run` path waits for a real PWM
  sysfs node. `tests/controller_dry_run_shim.py` bypasses only that wait so the
  deployed parser/state/output path can be tested without hardware mutation.

The two old camera-test photos identified by the user as invalid were not used.
The offline source is `data/test_source.jpg`, copied from the repository's
OpenCV DNN example, and its generated 640x640 NV12 input is
`data/test_input.nv12`.

## 120-degree 2K camera constraint

The planned camera is 120-degree, 2K, wide-angle and advertised as
distortion-free.  At stage J, enumerate the actual UVC node by `ID_PATH`, record
its exact width/height/pixel format/fps, preserve aspect ratio, and letterbox to
the OM's 640x640 input.  “2K” is not treated as an exact resolution until the
device reports it.  The 120-degree FOV type and residual lens distortion must
be verified with the physical unit; accepted intrinsics/horizon and camera
mount height/pitch must replace defaults before distance acceptance.  The
camera script is deliberately locked by `ENABLE_STAGE_J=0` and
`CALIBRATION_ACCEPTED=0`.

## Build in WSL2

From Windows PowerShell:

```powershell
wsl.exe -l -v
wsl.exe bash -lc "uname -a && pwd && cat /etc/os-release"
$wslRepo = wsl.exe wslpath -a 'C:\Users\ASUS\Desktop\ss928'
wsl.exe bash -lc "cd '$wslRepo/work/ss928_yolov8_vot' && make clean && make all"
wsl.exe bash -lc "cd '$wslRepo/work/ss928_yolov8_vot' && file bin/ss928_yolov8_vot && readelf -h bin/ss928_yolov8_vot | grep Machine && readelf -d bin/ss928_yolov8_vot | grep NEEDED"
```

The toolchain is
`/opt/linux/x86-arm/aarch64-mix210-linux/bin/aarch64-mix210-linux-` and the
headers/stub library come from the repository's matching V2.0.2.2 SDK.

## Offline board run

```sh
cd /root/smartbag_alert/runtime/yolov8_vot
chmod +x scripts/*.sh
./scripts/run_offline.sh 100 >logs/offline_stdout.jsonl 2>logs/offline_stderr.txt
```

`stdout` is reserved for alert JSONL.  A static or non-detected frame correctly
emits no alert record; diagnostics go to `stderr`.

## Camera command (phase-gated, not currently runnable)

After stage G passes, attach the intended unit, run `scripts/inspect_camera.sh`,
record the real mode and calibration in `config/detector.env`, then explicitly
unlock both gates.  The current offline-stage binary still rejects
`--source camera`; camera implementation and validation remain incomplete.

See `logs/TEST_RESULTS.md` and the absolute board log paths recorded there.
