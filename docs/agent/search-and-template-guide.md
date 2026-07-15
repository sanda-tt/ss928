# 查找策略与最小代码模板

> 不确定从哪里找代码、应选哪个 Sample 或资料冲突时读取。

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
