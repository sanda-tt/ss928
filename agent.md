# SS928 / Hi3403 / SD3403 开发资料 Agent 导航

本文件是给后续 AI agent 使用的项目上下文。目标不是替代原始资料，而是让 agent 在帮我写代码、改 Sample、查 SDK、查硬件连接时，能快速定位正确文件。

## 绝对规则

- 不要删除、精简、移动、重命名本目录下任何原始资料、压缩包、镜像、PDF、SDK、Sample、硬件资料。
- 需要写代码时，优先复制已有 Sample 的结构另建工程或在用户明确指定的工作目录中改动；不要直接破坏原始 SDK/Sample。
- 板子常见命名是同一平台：`Hi3403V100` 是官方 SoC 平台名，`SD3403V100` 常见于板卡丝印或行业标注，`SS928V100` 是开发板/SDK/软件包常用代号。
- 用户当前已经烧录 `Ubuntu 22.04`，后续板端应用、MPP Sample、外设例程优先按 Ubuntu/Linux 板端运行来考虑。
- 先查本地资料。本地没有时再查这些在线入口：`https://www.ebaina.com/tags/180000001969/24?page=1`、`https://gitee.com/hieulerpi`、`https://gitee.com/HiSpark`、`https://developers.hisilicon.com/forum/0155201230363076006`。
- 工作区域在work文件夹，在此新建文件

## Skill 使用指南

每个项目、调试或阶段性任务开始前，必须先做 skill 预检，不要直接进入全仓搜索或写代码：

1. 先查看当前 Codex 可用 skills 和本仓库 `skills/` 目录。只要任务场景命中某个 skill 的 `description` 或本节触发场景，必须先完整阅读对应 `SKILL.md`，再开始操作。
2. 如果用户点名某个 skill，优先按该 skill 执行；如果当前环境不能使用它，要先说明原因，再选择最接近的替代流程。
3. 新建任何 skill 时，必须同步更新本节的“本项目已有 skill 触发场景”，写清 skill 名、什么时候用、关键入口或默认安全边界。不能只把 skill 放进目录而不登记。
4. 涉及板端、网络、蓝牙、5G、外设或长时间运行程序时，按对应 skill 的安全规则先做只读探测，再上传、启动、停止或改配置。
5. 项目结束后，如果沉淀了可复用流程，除了新增或更新 skill，也要在“项目工作总结”中记录目标、验证结果和踩坑。

本项目已有 skill 触发场景：

- `skills/ss928-direct-board-debug/SKILL.md`：只要需要直接连接 SS928/HiEuler Pi 板子调试，就必须先用。包括 SSH/SFTP、上传文件、运行板端命令、启动或停止程序、查看日志、BMI270/BLE/外设板端联调、确认 `root@192.168.1.168` 在线状态。默认先做只读探测，优先以太网 SSH，串口只作为 SSH 失败或看启动日志时的后备。
- `skills/ss928-mt5710-5g-validation/SKILL.md`：测试或排查 TD Tech `MT5710/MT571X` 5G RedCap 模组时使用。包括 USB 枚举、`/dev/ttyUSB1` PCUI AT 指令、SIM/APN、蜂窝拨号、互联网连通、语音呼叫、GNSS/定位状态和 MT5710 专属问题。
- `skills/miniprogram-development/SKILL.md`：开发、修改、调试、预览、发布微信小程序时使用。包括页面、组件、`tabBar`、路由、图标资源、`project.config.json`、`appid`、真机预览、WeChat Developer Tools、`miniprogram-ci`、CloudBase/云开发相关流程。
- `skills/wechat-miniprogram-native/SKILL.md`：原生 JavaScript 微信小程序实现细节时使用，尤其是性能、包体、`setData` 数据路径、WXML/WXSS、原生组件兼容和避免 TypeScript/Taro/Uni-app 等跨端框架。

## 最高频入口

- 快速资料总览：`01. 快速使用指南【必看】/海鸥派2.0 资料目录介绍.pdf`
- 产品规格：`01. 快速使用指南【必看】/产品规格书/海鸥派Euler Pi产品规格书V2.0.pdf`
- 40Pin 复用表：`01. 快速使用指南【必看】/产品规格书/预留IO引脚复用图.xlsx`
- Ubuntu 环境：`05. 开发环境搭建/04.Ubuntu开发环境搭建指导手册.pdf`
- 原厂 SDK 快速编译：`06. 开发板源码编译/01.基于海思原厂SDK快速编译/基于海思原厂SDK快速编译指导手册.pdf`
- 适配后 SDK：`06. 开发板源码编译/02.适配后的SDK源文件/SS928V100_SDK_V2.0.2.2.tar/SS928V100_SDK_V2.0.2.2`
- 官方/适配 MPP Sample：`06. 开发板源码编译/03.MPP Sample例程包/SS928V100_SDK_V2.0.2.2_MPP_Sample-master`
- 更新的在线 MPP Sample 参考：`在线仓库/SS928V100_SDK_V2.0.2.3_MPP_Sample-master`
- 外设应用例程：`06. 开发板源码编译/04.外设例程包/HiEuler_PI_Peripherals_Sample-master`
- 外设驱动：`06. 开发板源码编译/05.外设驱动包/HiEuler_PI_Peripherals_Driver-master`
- 固件打包工程：`06. 开发板源码编译/06.固件打包工程/HiEuler_PI_Firmware_Building-master`
- 硬件原理图和外设资料：`07硬件资料`

注意：根目录有一个 `06.` 目录，当前看起来是空/异常入口，不要把它当成源码编译资料入口。正确入口是 `06. 开发板源码编译`。

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

## Sensor / 摄像头注意事项

Sensor 类型在 `src/Makefile.param` 里改：

- `SENSOR0_TYPE ?= SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT`
- `SENSOR1_TYPE ?= ...`
- `SENSOR2_TYPE ?= ...`
- `SENSOR3_TYPE ?= ...`

已列出的 Sensor 类型包括 `HY_S0603`、`OV_OS08A20`、`SONY_IMX347`、`OV_OS05A10`、`SONY_IMX485`、`OV_OS04A10`、`OV_OS08B10`、`SC450AI`、`SONY_IMX219` 等。

转接板和 I2C 对应关系来自 MPP Sample README：

- `EULER_1R2D V1.0`：sensor0 4lane 使用 I2C5；dToF 2lane 可在 J3/I2C4 或 J4/I2C5。
- `EULER_2R V1.0`：J3 是 sensor0 4lane/I2C5，J4 是 sensor1 4lane/I2C7。
- `EULER_4SEN V1.0`：四路 2lane，sensor0/I2C7，sensor1/I2C5，sensor2/I2C4，sensor3/I2C6。

Sensor 支持组合：

- IMX347：1R2D 支持 4lane sensor0；2R 支持 sensor0~1；4SEN 支持 2lane sensor0~3。
- OS04A10、OS08A20：1R2D/2R 支持 4lane；4SEN 不支持。
- SC450AI：1R2D/2R 支持 4lane；4SEN 支持 2lane sensor0~3。

当前用户实物连接：

- MIPI 显示屏是 `800x1280`。
- Sensor 转接板是 `EULER_4SEN V1.0`，也就是 4x2lane VIO。
- 目前只在 `sensor0` 接了一个 IMX347 摄像头；按 4SEN 资料，sensor0 对应 `I2C7`，Sensor 类型应优先用 `SONY_IMX347_2L_SLAVE_MIPI_2M_30FPS_12BIT`，lane divide 参考更新 sample 的四路 2lane 配置用 `LANE_DIVIDE_MODE_3`。

Sensor 时钟：

- 可改 `load_ss928v100` 脚本参数，例如 `./load_ss928v100 -i -sensor0 os08a20 -sensor1 os08a20 -sensor2 os08a20 -sensor3 os08a20`。
- 也可用寄存器：Sensor0 `bspmm 0x11018440 ...`，Sensor1 `0x11018460`，Sensor2 `0x11018480`，Sensor3 `0x110184A0`。
- IMX347 时钟配置值 `0x8001`，37.125MHz。
- OS04A10/OS08A20 时钟配置值 `0x4001`，24MHz。
- SC450AI 时钟配置值 `0xA001`，27MHz。

硬件资料位置：

- 摄像头模块：`07硬件资料/03.外设模块资料/07.摄像头模组`
- Sensor 转接板：`07硬件资料/03.外设模块资料/07.摄像头模组/05.Sensor转接板`
- dToF：`07硬件资料/03.外设模块资料/04.dToF`
- MIPI 显示屏：`07硬件资料/03.外设模块资料/05.MIPI显示屏`

## 外设代码怎么找

外设应用例程在 `HiEuler_PI_Peripherals_Sample-master`：

- `i2c_oled`：OLED 应用层控制。
- `mcu_tool`：控制海鸥派底板 MCU。
- `pwm`：PWM 舵机例程。
- `uart`：UART/RS485 读写例程。

外设驱动在 `HiEuler_PI_Peripherals_Driver-master`：

- `gpio`：内核态 GPIO 控制，默认示例是 40Pin Pin13/GPIO2_1。
- `hi_adc`：ADC 驱动，40Pin ADC 是 `LSADC_CH3`。
- `i2c_soft`：GPIO 模拟 I2C，四路 Sensor 时 I2C 不够用可参考。
- `oled`：OLED I2C 驱动。
- `RM500U`：移远 5G PCIe 模块驱动。
- `Tsensor`：SS928V100 芯片温度传感器，加载后 `cat /proc/Tsensor`。
- `ws73_sdk_linux_WS73_1.10.111`：WS73 星闪/蓝牙/Wi-Fi 三合一模块驱动。

40Pin 关键引脚来自 `预留IO引脚复用图.xlsx`：

- Pin3：`I2C0_SDA` / `GPIO11_4`，寄存器 `0x102F013c`。
- Pin5：`I2C0_SCL` / `GPIO11_5`，寄存器 `0x102F0140`。
- Pin11：`LSADC_CH3` / `GPIO10_0` / `PCIE_RST_N`，寄存器 `0x102F00FC`。
- Pin13：`GPIO2_1` / `USB_0_PWREN`，寄存器 `0x10230044`。
- Pin32：`PWM0_OUT1_0_P` / `GPIO17_0`，寄存器 `0x102F01EC`。PWM 例程使用 `bspmm 0x102f01ec 0x1201`。
- UART/RS485 例程提到 UART3 复用：`bspmm 0x102f012c 0x1201`、`bspmm 0x102f0130 0x1201`。

## 固件、系统和板端运行

已知账号信息：

- 提供的虚拟机账号：`ebaina`，密码 `ebaina`，适用于虚拟机登录、SSH、Samba/SMB。
- 固件打包工程 README 写的板端系统账号：`root`，密码 `ebaina`。
- 默认 IP：`192.168.1.168`。

虚拟机工作目录规划来自 `05. 开发环境搭建/02.虚拟机/虚拟机说明.txt`：

- `/home/ebaina/Workspace/HiEulerPi/SS928V100_SDK_V2.0.2.2`
- `/home/ebaina/Workspace/HiEulerPi/SS928V100_SDK_V2.0.2.2_MPP_Sample`
- `/home/ebaina/Workspace/HiEulerPi/HiEuler_PI_Peripherals_Sample`
- `/home/ebaina/Workspace/HiEulerPi/HiEuler_PI_Peripherals_Driver`
- `/home/ebaina/Workspace/HiEulerPi/HiEuler_PI_Firmware_Building`
- `/home/ebaina/Workspace/NFS`
- `/home/ebaina/Workspace/Tools`

固件打包工程：

- 路径：`06. 开发板源码编译/06.固件打包工程/HiEuler_PI_Firmware_Building-master`
- 脚本：`Generate_Image.sh`
- 支持 `Linux`、`Linux6.6`、`Ubuntu`、`openEuler`、`openHarmony`。
- 常用动作：生成固件 `-g`，删除生成文件 `-r`，用 `-os` 指定系统，例如 Ubuntu。
- 目录含义：`kernel` 放内核 `uImage_ss928v100`，`rootfs` 放文件系统，`opt` 放最终进 `/opt` 的用户程序，`overlay` 覆盖文件系统，`uboot` 放 boot image/env/分区表/logo，`out` 是输出。
- 4GB/8GB 以及 1-rank/2-rank DDR 对应不同 `boot_image_*`、`uboot_env_*`、`burn_table_*`，不要混用。

烧录/镜像资料：

- 固件烧写：`03. 开发板镜像烧录说明/02.海鸥派ToolPlatform固件烧写手册.pdf`
- SD 卡启动升级：`03. 开发板镜像烧录说明/03.海鸥派SD卡启动与升级.pdf`
- 常见问题：`03. 开发板镜像烧录说明/04.烧写常见问题汇总手册.pdf`
- Ubuntu 出厂镜像：`03. 开发板镜像烧录说明/01.出厂固件包/03.Ubuntu/Eulerpi_Ubuntu_IMAGE`

## 完整 SDK 结构

适配 SDK 根目录：`06. 开发板源码编译/02.适配后的SDK源文件/SS928V100_SDK_V2.0.2.2.tar/SS928V100_SDK_V2.0.2.2`

- `open_source`：Linux kernel、mbedtls、optee、e2fsprogs、util-linux 等开源组件。
- `osdrv`：SDK 构建、uboot/kernel/rootfs 工具链整合，readme_cn.txt 说明 `make BOOT_MEDIA=emmc CHIP=ss928v100 all` 等命令。
- `smp/a55_linux/interdrv`：内部驱动。
- `smp/a55_linux/osal`：OS 抽象层。
- `smp/a55_linux/mpp`：媒体处理平台核心，含 `sample`、`out/lib`、`out/ko`、`tools`。
- `smp/a55_linux/mpp/tools`：dump/调试工具，如 `vi_chn_dump.c`、`vpss_chn_dump.c`、`vo_chn_dump.c`、`ai_dump.c`、`ao_dump.c`。

SDK 补丁：

- 路径：`06. 开发板源码编译/08.补丁文件`
- 说明：`补丁使用说明.txt`
- 步骤核心：在 SDK 根目录创建 `hieulerpi_adapt`，放入 `adapt.sh`、`ebaina_hieulerpi_adapt.patch`、`logo.bin`、DDR xlsm 文件，然后执行 `./adapt.sh`，最后在 `osdrv` 下 `make BOOT_MEDIA=emmc CHIP=ss928v100 all`。

## 进阶资料

- OpenCV 移植：`09. 进阶功能开发/02.Opencv移植`，含 OpenCV 4.5.5 源码和 `SS928V100_OPENCV_4.5.5_aarch64-mix210-linux.tar.xz`。
- Qt：`09. 进阶功能开发/03.QT应用开发`，含 Qt 5.15.16 移植包。
- YOLOv8：`09. 进阶功能开发/04.Yolov8移植`，含模型转换、训练、部署 PDF。
- Python 调海思 API：`09. 进阶功能开发/06.Python调用海思API`，含 `pymodule` 源码、`python_vio` 可执行程序、`sample_pymodule.so`。
- Ubuntu-base 移植：`09. 进阶功能开发/05.Ubuntu-base移植/Ubuntu-base(20.04或22.04)移植.pdf`。
- openEuler：`09. 进阶功能开发/07.OpenEuler移植` 和 `08.OpenEuler-XFCE桌面即用体验指南`。

综合案例：

- `10. 进阶综合案例/01.QT+IVE菜单配置`：Qt + IVE，说明中提到 linxfb 为 `1920x540`，起始 `(0,540)`，默认 Sensor 是 IMX347，可在 pro 文件改。
- `10. 进阶综合案例/02.Sensor+3D dToF画中画`：可替换 MPP Sample 里的 dtof 例程，默认 IMX347。
- `10. 进阶综合案例/03.人车非目标算法识别`：旷视算法适配指南和演示视频。
- `10. 进阶综合案例/05.ModelZoo/model`：已转换好的 `.om` 模型，包括 ResNet50、YOLO11s、UNet、FaceNet、PaddleOCRv4-rec、HRNet、LRStereo-B、SuperPoint、CrowdCount、VDSR 等。

## 在线仓库本地副本

`在线仓库` 下已经有若干 Gitee 仓库的压缩包和展开目录：

- `SS928V100_SDK_V2.0.2.2_MPP_Sample-master`、`SS928V100_SDK_V2.0.2.3_MPP_Sample-master`：独立 MPP Sample。
- `ss928v100_gcc-master`：GCC/glibc 方向 SDK 仓，README 写明工具链 gcc 12.3.0、glibc 2.38。
- `ss928v100_clang-master`：CLANG/musl 方向 SDK 仓。
- `pegasus-master`：HiSpark Pegasus，总目录含 `docs`、`os/OpenHarmony`、`platform/ss928v100_gcc`、`platform/ss928v100_clang`、`vendor`。它的 README 明确：SS928/SS927 提供音视频采集、编解码、输出、视频前处理、日志等基础能力，也有海鸥派 Euler Pi、LubanCat-Hi3403 等厂商资料。
- `HiEuler_PI_Firmware_Building-master`：固件打包。
- `HiEuler_PI_Peripherals_Sample-master`：外设例程。
- `HiEuler_PI_ModelZoo-master`：海鸥派 ModelZoo 应用文档、脚本、源码。
- `modelzoo-master`：HiSpark ModelZoo。
- `hispark_ai-master`：HiSpark AI 工具链/样例，偏 WS63/RISC-V 轻量 AI，不是 SS928 MPP 主入口。

## 给后续 agent 的查找策略

写代码前先问自己是哪类任务：

- 摄像头/视频流/编码/显示：从 `SS928V100_*_MPP_Sample-master/src/vio`、`src/venc`、`src/vo`、`src/common` 查。
- HDMI/MIPI 屏：查 `src/vo`、`src/vdec`、`sample_comm_vo.c`，再查硬件 `07硬件资料/03.外设模块资料/05.MIPI显示屏`。
- Sensor 不出图：查 `src/Makefile.param` 的 `SENSORx_TYPE`、MPP README 的 Sensor 时钟、转接板 I2C、`07硬件资料/03.外设模块资料/07.摄像头模组`。
- NPU/AI/模型：查 `src/svp`、`09. 进阶功能开发/04.Yolov8移植`、`10. 进阶综合案例/05.ModelZoo`、`在线仓库/HiEuler_PI_ModelZoo-master`。
- Python 调 MPP：查 `09. 进阶功能开发/06.Python调用海思API`。
- GPIO/I2C/PWM/UART/ADC/温度/Wi-Fi/蓝牙/星闪/5G：查外设 Sample 和 Driver，再查 40Pin xlsx 和硬件资料。
- 改内核/uboot/rootfs/打包镜像：查完整 SDK 的 `osdrv/readme_cn.txt`、`06.固件打包工程`、`08.补丁文件`。

推荐搜索命令思路：

- 找函数实现：`rg -n "sample_comm_vi_start_vi|ss_mpi_vi|ss_mpi_vpss|ss_mpi_venc" 在线仓库/SS928V100_SDK_V2.0.2.3_MPP_Sample-master/src`
- 找 Sensor：`rg -n "SENSOR0_TYPE|IMX347|OS08A20|SC450AI|load_ss928v100" 在线仓库/SS928V100_SDK_V2.0.2.3_MPP_Sample-master`
- 找外设寄存器：`rg -n "bspmm|GPIO|PWM|UART|LSADC" "06. 开发板源码编译" "07硬件资料" "在线仓库"`
- 找板端运行说明：优先读各仓 `README.md`、`readme.txt`，再读 PDF。

## 最小代码模板选择

- 要做摄像头采集并 HDMI 显示：复制/裁剪 `src/vio/sample_vio.c`。
- 要做摄像头采集并保存 H.264/H.265：从 `src/vio` 中保留 VI->VPSS->VENC，参考 `src/venc` 的编码参数。
- 要做图像算法前处理：从 VI/VPSS 拿帧后参考 `src/svp/ive` 或 `src/opencv`。
- 要做 RTSP：从 `src/rtspserver` 和 VENC 取流逻辑组合。
- 要做简单 GPIO/PWM/UART：先看外设 Sample，不要用 MPP。
- 要做驱动：先看外设 Driver 的 Makefile.param，它默认 `ARCH=arm64`、`CROSS_COMPILE=aarch64-mix210-linux-`，`KERNELDIR` 指向适配 SDK 的 Linux 4.19 内核目录。

如果资料冲突：以“用户当前 Ubuntu22.04 + 本地适配 SDK V2.0.2.2 + 本地 MPP Sample”为第一优先级；在线 V2.0.2.3 和 Pegasus 作为更新参考。

## 项目工作总结

后续 agent 完成一个独立项目或阶段性调试后，必须在本节末尾追加工作总结。总结要写清楚：目标、最终可用方案、关键文件、验证结果、踩过的坑和不要再走的错误路线。不要把项目总结散落到硬件注意事项、编译要点或查找策略里。

### work/imx347_mipi_preview

- 目标：让 EULER_4SEN V1.0 转接板 sensor0 上的 IMX347 实时预览到 800x1280 MIPI 屏。
- 硬件事实：只接了 sensor0；IMX347 2lane；sensor0 对应 I2C7；lane divide 使用 `LANE_DIVIDE_MODE_3`；sensor 类型使用 `SONY_IMX347_2L_SLAVE_MIPI_2M_30FPS_12BIT`。
- 编译路径：用户在 WSL2 里从 `/mnt/c/Users/ASUS/Desktop/ss928/work/imx347_mipi_preview` 执行 `make`；Makefile 默认 `MPP_SAMPLE_ROOT=../../在线仓库/SS928V100_SDK_V2.0.2.2_MPP_Sample-master`，不要写 `/home/ebaina/Workspace/...`。
- 最终跑通链路：系统模式用 `OT_VI_OFFLINE_VPSS_ONLINE`，媒体链路为 `VI -> VPSS -> VO -> MIPI TX`。只启动 sensor0，但参考 SDK `src/vio/sample_vio.c` 的 EULER_4SEN 四路 2lane 思路，不要优先退回直连 `VI -> VO`。
- 显示方案：VPSS 缩放输出 `1280x720`，VO 通道 `OT_ROTATION_90` 后以 `720x1280` 居中显示到 `800x1280` MIPI 屏。
- 已验证：`sample_vdec 5` 可显示，证明 MIPI 屏和 MIPI TX 初始化表正常；`imx347_mipi_preview` 最终可显示实时摄像头画面，运行不卡。
- 踩坑：VPSS 通道旋转实测失败 `0xa007800d`，不要再优先尝试 `ss_mpi_vpss_set_chn_rotation()`；应使用 VO 通道旋转 `ss_mpi_vo_set_chn_rotation()`。
- 调焦：IMX347 当前按手动对焦处理，sample 未实现自动对焦控制；用户已通过手动对焦解决模糊问题。
- 诊断结论：早期 VI/VPSS 取帧探针在 `VI offline -> VPSS online` 模式下会打印失败，但不代表显示链路失败；最终以 VO/屏幕实际显示为准，后续不应保留这些误导性探针日志。

### work/bmi270_i2c_pose

- 目标：把新增补充资料里的 BMI270 六轴模块接到 SS928/HiEuler Pi，并在电脑端显示姿态数据。
- 资料结论：补充资料的 STM32 例程默认是 SPI，README 提醒姿态解算要按固定频率执行，示例为 200Hz；BMI270 数据手册确认芯片上电默认 I2C，CS/CSB 拉到 VDDIO 保持 I2C，SDO 决定 I2C 地址，SDO=GND 为 `0x68`，SDO=VDDIO 为 `0x69`。
- 推荐接法：VCC 接 40Pin Pin1/17 的 3.3V，GND 接任意 GND，SDA 接 Pin3 `I2C0_SDA`，SCL 接 Pin5 `I2C0_SCL`，CS 接 3.3V，SDO 默认接 GND，INT1 可接 Pin13 `GPIO2_1`，INT2 第一版先不接；如果需要第二路中断，可选空闲 GPIO 如 Pin15 `GPIO0_4`。
- 关键文件：`work/bmi270_i2c_pose/bmi270_i2c_pose.c` 是 Linux userspace I2C 读取和姿态解算程序；`viewer.html` 是电脑端 Web Serial 可视化；`pinmux_i2c0.sh` 设置 40Pin Pin3/Pin5 为 I2C0；`README.md` 写了接线、编译、运行和显示方式。
- 引脚复用：I2C0 复用值来自 SDK `smp/a55_linux/interdrv/sysconfig/pin_mux.c`，Pin3/Pin5 对应 `bspmm 0x102F013c 0x2031`、`bspmm 0x102F0140 0x2031`；INT1 可用 `bspmm 0x10230044 0x1200` 切到 GPIO2_1。
- 显示方案：板端程序输出 CSV，前三列固定为 `pitch,roll,yaw`，后面附带加速度、角速度、温度和时间；电脑可以直接用串口终端看文本，也可以用 Chrome/Edge 打开 `viewer.html` 选择串口后看 3D 立方体姿态。
- 验证结果：本机 Windows 没有 `gcc`，WSL 没有可用发行版，所以未在本机编译板端 C 程序；已确认 `BMI270_config.h` 本地存在，`viewer.html` 的脚本语法通过 Node 检查。内置浏览器因本地访问策略拦截 `file://`/localhost，未完成实际渲染截图。
- 踩坑：不要把 BMI270 的 VCC 或任何信号接 5V；第一版不需要中断脚，先轮询读通 I2C；如果通过 SSH 运行，CSV 会走 SSH 终端，不会自动进入浏览器串口可视化，要么在串口控制台运行，要么后续改 WebSocket/UDP。

### work/linux_bmi270_backpack + work/ssminiprogram

- 目标：先打通 SS928 读取 BMI270 姿态数据并通过 BLE 发到微信小程序实时展示的最小链路。
- 板端方案：复用 `work/linux_bmi270_backpack/bmi270_backpack.py`，它输出紧凑 JSON，并作为 Nordic UART Service BLE 外设广播 `BMI270-Backpack`；新增 `config.ss928_ble.json` 和 `start_ss928_ble.sh`。
- 接线基准：沿用 `work/bmi270_i2c_pose` 的旧接线，SDA=Pin3/I2C0_SDA，SCL=Pin5/I2C0_SCL，CS/CSB 拉 3.3V，SDO 接 GND，地址 `0x68`，对应 `/dev/i2c-0`。不要把通用 Python 示例里的 I2C1 当成这块板当前接线。
- BLE 启动：`start_ss928_ble.sh` 会先设置 I2C0 pinmux。当前板子实测系统 BlueZ 可用，脚本默认跳过 `ble.sh` 直接注册 GATT；只有没有系统控制器时才用 `START_BLE_STACK=1` 切到 `/opt/sample/ws73` 执行 `ble.sh 0` 并解析 DBus export 行。
- 小程序方案：`work/ssminiprogram/miniprogram/pages/index` 已从云开发 quickstart 首页改成原生微信 BLE 面板，流程为打开蓝牙、按 NUS UUID 扫描、连接、发现服务/特征、订阅 TX notify、按换行重组 BLE 分片并解析 JSON，展示 roll/pitch/yaw、六轴原始值、状态、告警、帧率，并可向 RX 写 `ZERO` / `STATUS`。
- 验证结果：Windows 本机已通过 `node --check` 检查小程序 JS，通过 `python -m json.tool` 检查相关 JSON，用 Git `sh -n` 检查启动脚本语法，并用 `--simulate --no-ble` 确认板端仍输出含 `r/p/y` 的 JSON。实际 I2C、BlueZ GATT 注册和微信真机 BLE 连接必须在 SS928 和手机上验证。
- 踩坑：微信开发者工具模拟器不能替代真机 BLE；如果小程序扫描不到，先用 nRF Connect 或 bluetoothctl 看板端是否真的广播 `BMI270-Backpack` 和 NUS UUID，再查 `ble.sh` 的 DBus 环境是否被 Python 进程继承。

### skills/ss928-mt5710-5g-validation

- 目标：把 SS928 上 TD Tech `MT5710/TDTECH_MT571X` 5G RedCap 模组的板端验证流程固化成可复用 skill。
- 关键文件：`skills/ss928-mt5710-5g-validation/SKILL.md`；元数据在 `skills/ss928-mt5710-5g-validation/agents/openai.yaml`。
- 已验证可用：装好外置天线后，移动 SIM 卡使用 `cmnet` 拨号成功，NCM 网口 DHCP 获取 `10.72.12.12`，运营商 DNS 为 `223.87.253.100`/`223.87.253.253`，`223.5.5.5` 和 `www.baidu.com` 均 5/5 包通过，说明流量上网和互联网连通已通。
- 已验证可用：`/dev/ttyUSB1` PCUI AT 通道正常，`ATD18180871724;` 语音呼叫信令可从外呼/振铃进入 active，`ATH` 可挂断；音频通路未接麦克风/喇叭或 PCM，因此只确认信令，不确认声音。
- 已验证不可用：当前 `MT5710_CN V100R001C00B095` Mini-PCIe 固件对 `AT^GNSSENABLE`、`AT^GNSSGETINFO`、`AT^GNSSGETNMEA` 以及常见 `CGPS/CGNS/QGPS` 命令均返回不支持，`/dev/ttyUSB3` 也无 NMEA 输出；定位/GNSS 目前按固件或产品变体限制处理。
- 踩坑：不要固定写死 `usb0`，本次实际 NCM 网口为 `enx7ecca5347f7b`；无天线时 `^HCSQ` 曾只有约 `-126 dBm` 且无法稳定注册，装天线后约 `-91 dBm` 并成功驻网拨号；不要把 RM500U/Quectel 命令直接套到 MT5710。
- SIM/APN 规则：当前实测是移动卡 `cmnet`；后续换电信普通数据卡优先用 `ctnet`，换联通普通数据卡优先用 `3gnet`，物联网卡或专网卡必须以运营商下发 APN 为准；APN 属于 SIM/套餐属性，不属于 MT5710 模组固定属性。
