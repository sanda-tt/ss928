# 项目历史与已验证结论索引

只在当前任务需要复用历史方案、核对验证结果或规避旧坑时读取相应文件。不要一次性加载整个目录。

- `imx347_mipi_preview.md`：IMX347、EULER_4SEN、VI→VPSS→VO→MIPI 屏实时预览与旋转方案。
- `bmi270_i2c_pose.md`：BMI270 I2C 接线、姿态解算、CSV 和 Web Serial 可视化。
- `linux_bmi270_backpack-and-ssminiprogram.md`：BMI270 板端 BLE 与微信小程序实时姿态链路。
- `ss928-mt5710-5g-validation.md`：MT5710 驻网、NCM、AT、语音和 GNSS 限制验证。
- `imu_fall_detector.md`：纯 Python 摔倒/撞击状态机、测试与调参注意事项。
- `ss928-max98357-audio-playback.md`：MAX98357 I2S、音频转换、pinmux 和 MPP Sample 播放。
- `dx_gp21_tracker-and-tracks.md`：DX-GP21-A GNSS、UART4、轨迹保存、BLE 与小程序地图。
- `ssminiprogram-cloud-track-display.md`：小程序经 CloudBase Event Function 读取并显示云端 GNSS 历史轨迹。
- `mr20-udp-pc-validation.md`：MR20 Windows UDP 接收、0x60B 目标帧解析与 CSV 日志验证。

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
