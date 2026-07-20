# SS928 Factory YOLOv8 VOT Design

## Goal

Build an isolated `work/ss928_yolov8_vot` AArch64 program that loads the unchanged factory model `/opt/sample/yolov8/yolov8n.om` through the V2.0.2.2 ACL runtime and closes the loop from offline input through detection, tracking, distance, relative velocity, collision risk, and `vision_alert` JSONL before enabling a single UVC camera.

## Fixed constraints

- The source of truth is `D:/Downloads/Codex_SS928_工厂YOLOv8_距离速度检测全流程指令.md` and its A-M phase gates.
- Reuse `vot.c`, `vot.h`, `vot_backend.cpp`, `vot_backend.h`, `test_core.c`, and `test_backend.cpp` from the existing `vision_obstacle_tracker/c_port`; do not rewrite the distance, velocity, stable-ID, or risk algorithms.
- Use the V2.0.2.2 SDK ACL headers and `lib/linux/hisilicon/npu/stub/libascendcl.so` with the SDK compiler `aarch64-mix210-linux-`; do not mix another CANN runtime.
- Do not retrain, reconvert, copy, overwrite, move, or modify the factory OM.
- Do not use NCNN, ONNX Runtime, Ultralytics, OpenMP, `-fopenmp`, or `libgomp` in the target program.
- Load the model once and create reusable input and output datasets/buffers once per process.
- Build in WSL2, prove AArch64 using `file` and `readelf`, and use the board only to inspect and run artifacts.
- Keep JSONL stdout clean; send diagnostics to stderr.

## Architecture

`FrameSource` produces a timestamped source frame or a model-ready raw buffer. `FramePreprocessor` applies a descriptor-selected 640x640 letterbox and emits the exact input bytes required by the OM. `Ss928AclYoloV8Detector` owns ACL initialization, the model, descriptor, and reusable datasets/buffers. `decode_yolov8` validates the descriptor-derived layout, selects one best class per anchor, maps boxes through independent `pad_x`/`pad_y`, and calls the existing class-aware serial NMS. The existing `BackendSimpleTracker` and C VOT core then assign raw/stable IDs, estimate distance and relative velocity, assess risk, stabilize warnings, and select one highest-risk event for JSONL.

Offline raw inference is the first runnable target. Image/video conversion is a host-side preparation path unless a verified board codec dependency is available. V4L2 camera capture is enabled only after the offline NPU, VOT, and JSONL gates pass; it uses a capacity-one latest-frame slot and capture-time `CLOCK_MONOTONIC` timestamps.

## Error and cleanup contract

Every ACL failure reports the stage, API name, return code, model path, and relevant tensor index/size. Initialization records completed ownership steps so shutdown can destroy buffers, datasets, descriptor, model, model memory, device, and ACL exactly once in reverse order. SIGINT/SIGTERM sets a stop flag and lets the normal cleanup path run.

## Verification gates

1. Inventory and unique cross-build route are recorded without board configuration changes.
2. `om_inspect` records names, dtypes, formats, dims, byte sizes, model memory, and weights as text and JSON.
3. Host/WSL tests cover copied VOT behavior, letterbox geometry, NV12 size/layout, decode layout, invalid floats, size mismatch, and class-aware NMS.
4. Board offline tests prove one real target frame and 100/100 executions with bounded boxes and finite output.
5. JSONL parses standalone and through the existing controller in dry-run mode.
6. Only then, live UVC runs for ten minutes with measured capture, inference, and end-to-end rates plus memory/resource checks.

