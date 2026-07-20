# SS928 工厂 YOLOv8 + VOT 离线闭环阶段记录

## 目标

在不修改、重训或重新转换工厂 OM 的前提下，建立独立
`work/ss928_yolov8_vot/` 工程，使用 V2.0.2.2 SDK 配套工具链封装 ACL
推理，并复用已有 VOT 距离、相对速度和风险算法。严格执行“离线真实检测
通过后才进入 USB”的阶段门禁。

## 环境事实

- WSL2 可用，交叉工具链为
  `/opt/linux/x86-arm/aarch64-mix210-linux/bin/aarch64-mix210-linux-`，GCC
  7.3.0；头文件和 ACL stub 来自仓库 V2.0.2.2 SDK。
- 板端没有完整编译器、Make/CMake，最终工程只在 WSL2 交叉编译，板端只
  加载 OM 和运行 AArch64 程序。
- 工厂 OM SHA256：
  `7010d928c2ce9675ea38b6ee353f1c4fcd4ec648a335d3735d85f2e0313d030b`。
- 输入：`images`，UINT8，NHWC，`[1,960,640,1]`，614400 bytes；静态
  AIPP 为 640x640 YUV420SP_U8/NV12，CSC 开启，`rbuv_swap=0`。
- 输出：`Concat_254:0:output0`，FLOAT32，ND，`[1,84,8400]`，
  2822400 bytes。

## 实现与关键文件

- `work/ss928_yolov8_vot/src/ss928_acl_detector.cpp`：ACL 初始化、一次加载、
  固定输入输出缓冲、缓存 flush/invalidate、执行和反序释放。
- `frame_preprocess.cpp`：等比例缩放、letterbox、工厂 AIPP 所需 NV12。
- `yolov8_postprocess.cpp`：`[1,84,8400]` 解码、类别过滤、反 letterbox、
  class-aware NMS。
- `vot_pipeline.cpp`：只负责把检测和 stable ID 送进原算法并形成 JSONL。
- `vot.c`、`vot_backend.cpp` 与原项目 SHA256 完全一致：分别为
  `4644646b85182c86c54c10a6c5e16157ac4a6f4e0323f3727e83d92b23030c92`
  和 `61989a30374a2e5442d99c0084f0245f64519be5c9f582204b74412d347464f2`。

## 真实验证结果

- WSL2 七项测试通过；二进制经 `file`/`readelf` 确认为 AArch64 ELF。
- 板端 100 帧：100/100 成功，模型加载 1 次，平均 NPU 25.112 ms，平均
  后处理 16.292 ms，端到端 23.348 FPS；返回 0，无新增 dmesg、无残留
  进程。
- `ldd` 全部解析，未依赖 `libgomp`。
- 合成检测输入下，原 VOT 输出 stable ID、距离、`vz < 0`、TTC、风险和
  干净 JSONL，证明软件连接和原算法复用成立。

## 未通过与未验证项

- 阶段 G 未通过：仓库内清晰双层大巴图在置信度 0.25 时为 0 个候选框；
  已知 COCO 大巴诊断图的目标类别最大分数也异常低。ACL 调用成功，输出
  无 NaN/Inf，因此不能把稳定运行冒充真实检测闭环成功。
- 最后复核 V2.0.2.2 原样例：它同样直接送 640x640
  `OT_SVP_IMG_TYPE_YUV420SP`，并按 channel-first 84x8400 概率解码且不再
  做 sigmoid；因此没有发现本工程漏掉的样例专用输入变换。
- 由于阶段门禁，USB 采集、latest-frame、10 分钟运行、2/4/6 m 实物标定
  和实时控制器集成均未开始。
- 用户计划使用 120°、2K、广角、标称无畸变 UVC 摄像头。后续必须先按
  `ID_PATH` 枚举真实节点和 UVC 模式，再验证 FOV 类型、残余畸变、内参、
  地平线、安装高度与俯仰；2K 画面需保持比例 letterbox 到 640x640。

## 踩坑和禁止重复路线

- 不要按普通 NCHW float 模型分配输入；真实契约是 614400-byte NV12。
- 不要加入 `-fopenmp` 或上传 `libgomp`。工厂 sample 自身因缺
  `libgomp.so.1` 启动失败，只是诊断证据，不能成为最终程序。
- 不要把 NV21、拉伸到 640x640、R/B 交换、降低阈值等已排除路线当作
  修复；这些诊断没有恢复目标类别响应。
- 现有控制器的 stock `--dry-run` 仍会等待 PWM sysfs 节点；测试 shim 只
  绕开该等待，不得将其描述为 stock dry-run 已通过。
- 阶段 G 未过之前，不得进入摄像头阶段或填写虚假的距离标定结果。

## 2026-07-20 阶段 G 三层隔离复核

- 固定唯一输入为 `work/ss928_yolov8_vot/data/test_input.nv12`，大小
  614400 字节，SHA256 为
  `76314d959c8e4dcd6a1d2893c9fad3013f31805caadcdcbc58caeda220a76e0`；
  参考 runner 与当前 runner 均直接读取该文件，不各自预处理。
- 板端实时查询确认 OM 输入为单个 `UINT8/NHWC [1,960,640,1]`，静态
  AIPP 接受 640x640 `YUV420SP_U8`，`rbuvSwap=0`（NV12 UV），无
  crop/resize/padding，启用 YUV 到 RGB CSC；输出为单个
  `FLOAT32/ND [1,84,8400]`，2822400 字节。
- 独立参考 runner 按 V2.0.2.2 官方 Sample 的 ACL 内存和数据集路径执行；
  当前 runner 只调用 `Ss928AclDetector`。两者原始输出 SHA256 均为
  `c843d68eded3eba19d326afede153f7647d8b081bb56e62f716bf7dd68f75298`，
  705600 个 float 全部逐元素相同，最大绝对误差为 0。
- 因此停止 MMZ/ACL 猜测并进入分支 A。原始布局确认是 `[1,84,8400]`，
  类别概率已经在 `[0,1]`，不应再次 sigmoid；当前转置、80 类、xywh、
  阈值、NMS 和 letterbox 还原均与工厂后处理契约一致。
- 固定大巴图的所有类别最大值仅 0.035400390625，car 最大值约
  0.00671768、bus 最大值约 0.00014317；在 0.25 阈值前即为 0 个候选，
  所以 NMS、坐标转换和 VOT 都尚未获得输入。零检测首次出现于工厂 OM 的
  原始类别响应，不是当前 ACL 封装或 YOLO 后处理制造的。
- 完整证据位于
  `work/ss928_yolov8_vot/diagnostics/STAGE_G_EVIDENCE.md` 及其
  `diagnostics/artifacts/`。阶段 G 的正检测门禁仍未通过，USB 阶段保持关闭。
