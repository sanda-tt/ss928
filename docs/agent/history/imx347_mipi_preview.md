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
