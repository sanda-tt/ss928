# MPP、摄像头与视频开发

> 涉及 VI、VPSS、VO、VENC、RTSP、编码、显示或摄像头媒体链路时读取。

## 写 MPP / 摄像头 / 视频代码时

优先从 MPP Sample 开始，不要凭空写海思媒体管线。

推荐参考顺序：

1. 新代码若要和已提供的适配 SDK 一起编译，优先使用 `06. 开发板源码编译/03.MPP Sample例程包/SS928V100_SDK_V2.0.2.2_MPP_Sample-master`。
2. 若要找更新功能或对照差异，再看 `在线仓库/SS928V100_SDK_V2.0.2.3_MPP_Sample-master`。
3. 如果独立 Sample 缺测试素材，去完整 SDK 中找原始目录：`SS928V100_SDK_V2.0.2.2/smp/a55_linux/mpp/sample/`。

MPP Sample 重要结构：

- `src/Makefile.param`：交叉编译器、OS_TYPE、Sensor 类型、include、lib 链接都在这里。
- `src/common/`：最重要的公共封装，写代码时优先复用这些函数。
- `src/common/sample_comm.h`：公共结构和函数声明。
- `src/common/sample_comm_sys.c`：VB/MMZ/系统初始化。
- `src/common/sample_comm_vi.c`：Sensor、MIPI RX、VI 初始化。
- `src/common/sample_comm_vpss.c`：VPSS 组和通道。
- `src/common/sample_comm_vo.c`：HDMI/MIPI TX/VO 输出。
- `src/common/sample_comm_venc.c`：编码、取流、保存码流。
- `src/common/sample_comm_audio.c`：音频输入输出和 AAC。
- `include/hisilicon/`：海思 API 头文件，直接 MPI API 是 `ss_mpi_*.h`，公共类型是 `ot_common_*.h`。
- `lib/linux/hisilicon/`：Linux/Ubuntu 侧用到的海思库，如 `libss_mpi.a`、`libss_hdmi.a`、`libsns_*.a`、`libss_ive.a`、`libss_aiv.a` 等。

典型媒体管线写法应仿照 `src/vio/sample_vio.c`：

1. 配置 VB/MMZ，调用 `sample_comm_sys_init_with_vb_supplement()` 或相关 sys init。
2. 根据 `SENSOR0_TYPE` 等获取输入尺寸：`sample_comm_vi_get_size_by_sns_type()`。
3. 初始化并启动 VI：`sample_comm_vi_get_default_vi_cfg()`、`sample_comm_vi_start_vi()`。
4. 启动 VPSS：`sample_comm_vpss_get_default_grp_attr()`、`sample_common_vpss_start()`。
5. 需要显示则启动 VO：`sample_comm_vo_start_vo()`，默认 HDMI 常见为 `OT_VO_OUT_1080P60`。
6. 需要编码则启动 VENC：`sample_comm_venc_start()`、`sample_comm_venc_start_get_stream()`。
7. 用 `sample_comm_vi_bind_vpss()`、`sample_comm_vpss_bind_vo()`、`sample_comm_vpss_bind_venc()` 绑定模块。
8. 退出时按反方向解绑、停止 VI/VPSS/VO/VENC，再 `sample_comm_sys_exit()`。

常用 Sample 目录：

- `src/vio`：摄像头输入、VPSS、VO/VENC 串联，是写视觉应用的第一参考。
- `src/venc`：编码 H.264/H.265/JPEG 等。
- `src/vdec`：解码和显示。
- `src/vo`：显示输出。
- `src/audio`：音频录放和 AAC。
- `src/host_uvc`、`src/uvc_app`：USB 摄像头/UVC。
- `src/svp`：IVE/NPU/SVP 类 AI 或视觉处理。
- `src/opencv`：OpenCV 结合示例。
- `src/rtspserver`：RTSP 推流。
- `src/sample_object_track`、`src/traffic_capture`：目标跟踪/交通类参考。

编译要点：

- 用户已经在 WSL2 里准备好交叉编译工具链；后续 agent 不要默认在 Windows/PowerShell 侧替用户编译，写好代码和 Makefile 后优先给出 WSL2/Linux 下的 `make` 命令，让用户在 WSL2 中编译。
- Makefile 里的 `MPP_SAMPLE_ROOT` 不能写资料示例路径或 agent 猜测路径，例如 `/home/ebaina/...`；必须写用户 WSL2 里真实存在的 SDK/Mpp Sample 根目录。若不知道真实路径，先让用户在 WSL2 里进入 SDK 目录执行 `pwd` 并把输出发来，再改 Makefile，不能擅自猜。
- 对 `work/imx347_mipi_preview` 这类位于 `work/` 下的独立 sample，默认从 WSL2 当前目录 `/mnt/c/Users/ASUS/Desktop/ss928/work/imx347_mipi_preview` 执行 `make`；可用无空格相对路径 `../../在线仓库/SS928V100_SDK_V2.0.2.2_MPP_Sample-master` 作为 `MPP_SAMPLE_ROOT`，不要再写 `/home/ebaina/Workspace/...`。
- Linux/Ubuntu 侧通常用 `OS_TYPE=linux`，工具链默认 `aarch64-mix210-linux-`。
- openEuler 侧才用 `OS_TYPE=openeuler` 和 `arm-openeuler-linux-gnueabi-`。
- 交叉工具链在 `05. 开发环境搭建/01.交叉编译工具/`，包括 `aarch64-mix210-linux.tgz`、`openeuler_gcc_arm64le.tar.gz`、`cc-riscv32-cfg5-musl-20211008-elf.tar.gz`。
- 编译全部 Sample：进入 MPP Sample 的 `src` 后执行 `make`。
- 编译单个 Sample：进入如 `src/vio` 后执行 `make`。
- 自己写 Makefile 时要保留 `-mcpu=cortex-a53`，这是 SDK readme 明确提醒的 SS928V100 编译选项。
