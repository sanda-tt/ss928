# 资料入口、SDK 结构与仓库地图

> 查找本地资料、SDK 目录、版本差异或在线仓库副本时读取。

## 外部在线入口

先查本地资料。本地没有时再查这些入口：

- `https://www.ebaina.com/tags/180000001969/24?page=1`
- `https://gitee.com/hieulerpi`
- `https://gitee.com/HiSpark`
- `https://developers.hisilicon.com/forum/0155201230363076006`

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
