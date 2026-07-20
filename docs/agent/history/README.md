# 项目历史与已验证结论索引

只在当前任务需要复用历史方案、核对验证结果或规避旧坑时读取相应文件。不要一次性加载整个目录。

- `imx347_mipi_preview.md`：IMX347、EULER_4SEN、VI→VPSS→VO→MIPI 屏实时预览与旋转方案。
- `bmi270_i2c_pose.md`：BMI270 I2C 接线、姿态解算、CSV 和 Web Serial 可视化。
- `linux_bmi270_backpack-and-ssminiprogram.md`：BMI270 板端 BLE 与微信小程序实时姿态链路。
- `ss928-mt5710-5g-validation.md`：MT5710 驻网、NCM、AT、语音和 GNSS 限制验证。
- `imu_fall_detector.md`：纯 Python 摔倒/撞击状态机、板端三条件融合、测试与调参注意事项。
- `ss928-max98357-audio-playback.md`：MAX98357 I2S、音频转换、pinmux 和 MPP Sample 播放。
- `dx_gp21_tracker-and-tracks.md`：DX-GP21-A GNSS、UART4、轨迹保存、BLE 与小程序地图。
- `ssminiprogram-cloud-track-display.md`：小程序经 CloudBase Event Function 读取并显示云端 GNSS 历史轨迹。
- `ssminiprogram-cloud-fall-alarm-display.md`：小程序经 CloudBase Event Function 读取并显示摔倒报警历史。
- `mr20-udp-pc-validation.md`：MR20 Windows UDP 接收、0x60B 目标帧解析与 CSV 日志验证。
- `mr20-smartbag-alert-integration.md`：MR20 网口雷达接入 SmartBag 四级右后预警、配置和板端网络验证。
- `tm6605-lra-haptic-alert.md`：TM6605 双 LRA 触觉驱动、I2C 复用与四级预警时序。
- `smartbag-pwm-light-modules.md`：双侧小信号光模块的 PWM 引脚分配及三级/四级光效时序。
- `ss928-new-board-migration-2026-07-20.md`: clean-board project restore, organized `/root` workspace, WS73 BlueZ startup, boot services, and persistent MR20 `eth1` network configuration.
- `smartbag-board-cloud-upload.md`：DX-GP21-A 轨迹、BMI270 处理后姿态/每日统计、最终摔倒信号的统一 CloudBase 上传和小程序回读闭环。
- `mt5710-smartbag-cloud-upload.md`：MT5710 电信 `ctnet` NCM 建链、5G 路由确认、完整 telemetry 上传和小程序服务层回读。

## 新增历史记录格式

每个独立项目使用一个文件，至少记录：

- 目标；
- 硬件或环境事实；
- 最终可用方案；
- 关键文件；
- 验证结果；
- 未验证项；
- 踩坑和不要再走的路线。

原始维护要求：后续 agent 完成一个独立项目或阶段性调试后，必须在本节末尾追加工作总结。总结要写清楚：目标、最终可用方案、关键文件、验证结果、踩过的坑和不要再走的错误路线。不要把项目总结散落到硬件注意事项、编译要点或查找策略里。
