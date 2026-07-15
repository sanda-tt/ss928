# 固件、系统与 SDK 构建

> 涉及虚拟机目录、固件、内核、uboot、rootfs、镜像、SDK 补丁或完整系统构建时读取。

## 账号信息

账号、密码和默认板端地址已迁移到仓库根目录 `.local/device-access.md`。该文件为本地私有文件，不得提交 Git。原始文本仍完整保存在 `docs/agent/archive/AGENTS.original.md`。

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
