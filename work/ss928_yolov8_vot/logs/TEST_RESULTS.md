# Real verification record — 2026-07-20

Board runtime root: `/root/smartbag_alert/runtime/yolov8_vot`

## Passed

- Seven native tests passed in WSL2: original C core, original C++ backend,
  preprocess, YOLO decode/NMS, model tensor metadata, ACL contract and VOT
  pipeline.
- Cross binary: ELF64 AArch64 PIE, `Machine: AArch64`.
- Board one-frame ACL call: success, output finite, no NaN/Inf.
- Board 100-frame run: `requested=100 successful=100 model_loads=1`, average
  inference `25.112 ms`, average postprocess `16.292 ms`, end-to-end
  `23.348 FPS`, exit 0, no new dmesg lines, no leftover process.
- Board dynamic dependencies: all found; `libgomp` absent.
  `LD_LIBRARY_PATH=/opt/lib:/opt/lib/npu` is required; without it the dynamic
  loader cannot locate `libascendcl.so`.
- Synthetic VOT result: stable track id 1, copied VOT distance 1.0237 m,
  relative approach speed 5.6302 m/s, `vz=-5.6302`, TTC 1.4 s, level 3,
  clean `vision_alert` JSONL.
- Controller parser/state/output via the documented dry-run sysfs-wait shim:
  consumed that JSONL, emitted left TM6605 and 50% left-light dry actions, and
  exited 0.  This is not recorded as a stock-controller dry-run pass.

## Failed / blocked

- Stage G real detection: 0 raw candidates and 0 detections on the repository
  double-decker-bus image at confidence 0.25.  This blocks the claimed
  NPU-detection -> tracking -> distance -> speed -> risk -> JSONL real loop.
- Final V2.0.2.2 sample-source cross-check confirmed that the factory path also
  feeds a direct 640x640 `OT_SVP_IMG_TYPE_YUV420SP` buffer and decodes the
  output as channel-first 84x8400 probabilities without an extra sigmoid.
  Our stretch/NV12 and layout diagnostics therefore did not reveal an omitted
  sample-specific transform.
- Raw board-output audit is saved separately: for the repository bus image the
  `bus` maximum is `0.000024140`, and for the official diagnostic bus image it
  is `0.000052273`; both are far below 0.25.  The global maximum incorrectly
  lands on class 79, so this is not caused by target filtering or NMS.
- The factory sample diagnostic could not start because that sample itself
  requires missing `libgomp.so.1`; it was not adopted and no library was added.
- Stock controller `--dry-run` exits before consuming JSON because its PWM
  adapter waits for a physical sysfs node even in dry-run mode.  A project test
  shim is provided to isolate that existing issue and exercise the real parser.
- Stage J USB, 10-minute live run and physical 2/4/6 m calibration were not
  entered due the strict stage-G gate.

## Board evidence

- `logs/00_inventory.txt`
- `logs/01_om_inspect_ldd.txt`
- `logs/01_model_desc.txt`
- `logs/model_desc.json`
- `logs/03_offline_100.txt`
- `logs/03_offline_100_stdout.jsonl`
- `logs/03_offline_100_summary.txt`
- `logs/03_dmesg_delta.txt`
- `logs/03_ldd.txt`
- `logs/04_controller_dry_run.txt`
- `logs/04_controller_dry_run_shim.txt`
- `logs/05_final_script_stdout.jsonl`
- `logs/05_final_script_stderr.txt`
- `logs/05_camera_gate_stderr.txt`
- `logs/05_final_scripts_summary.txt`
- `logs/06_final_ldd.txt`
- `logs/diag_factory_sample_sc450ai.txt`
- `logs/diag_target_class_maxima.txt`

All paths above are under
`/root/smartbag_alert/runtime/yolov8_vot/` on the board.
