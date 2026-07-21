# SS928 yolo11n SVP-NNN OM board compatibility check

Date: 2026-07-21

## Goal

Upload and directly test these supplied models on the physical SS928 board NPU:

- `补充资料/yolo11n_imgsz640_svp_nnn.om`
- `补充资料/yolo11n_imgsz512_svp_nnn.om`

## Board facts

- Board identity: `localhost.localdomain`, Linux `4.19.90`, `aarch64`.
- The NPU kernel modules are loaded: `ot_npu_tsfw`, `ot_npu_dfx`, `ot_npu_aicpu`, and `ot_npu_device`.
- The board ACL runtime is `/opt/lib/npu/libascendcl.so`.

## Files and transfer proof

Both files were transferred to `/root/ss928_yolo11n_model_check/` using a small-block SFTP transfer after the normal SFTP upload was reset by the board. Board SHA-256 values matched the local files exactly:

- `yolo11n_imgsz640_svp_nnn.om`: `91c664f85119a868a0adf0c4d7434ad93e449a90f295a414ecc46236e323cb7e`
- `yolo11n_imgsz512_svp_nnn.om`: `96d413782c15f876a5b7f1c069e02ce33806214cbd4415fbfa89aaec6b1333c8`

## Result

Neither supplied model is runnable by the board's current ACL/NPU runtime. The independent `om_inspect` program reached ACL device mode, then both models failed at `aclmdlQuerySize` before a tensor descriptor or inference could be created:

```text
aclmdlQuerySize ret=145000
EH9999: Inner Error! [Get][MemAndWeightSize]query size failed
```

This is a model/runtime compatibility failure, not a transfer corruption or a general NPU outage. As a control, the same ACL runner successfully loaded and executed `/opt/sample/yolov8/yolov8n.om` on a real 614400-byte NV12 input, producing its expected 2822400-byte `[1,84,8400]` output tensor (SHA-256 `c843d68e...68f75298`).

## Next requirement

Obtain an `.om` exported for this board's installed SS928 NPU/ACL runtime (or the matching runtime and compatible SDK/firmware). Do not try to force either current file through preprocessing or postprocessing: the failure occurs before input binding.

## Additional model checked

The supplied `补充资料/yolo11s.om` was also transferred to the same board test directory on 2026-07-21. Its 10,368,919-byte board copy matched local SHA-256 `19415b173c90122089f0a63d1cb54d9eba2904905438a980cb4e44f17c8981c0`, but it failed identically at `aclmdlQuerySize` with `ret=145000 / EH9999`. It is not runnable with the current SS928 ACL runtime either.

## Do not repeat

- Do not treat a successful local file copy as an NPU compatibility result; run `aclmdlQuerySize` and an actual `aclmdlExecute` control.
- Do not replace the factory model under `/opt/sample/`; use the separate `/root/ss928_yolo11n_model_check/` test directory.
