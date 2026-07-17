# SS928 repository agent rules

本文件只保留全仓库必须常驻的规则和知识路由。详细资料位于 `docs/agent/`，只在任务命中时按需读取，禁止无目的地一次性加载整个知识库。

## 1. 全局安全边界

- 不要删除、精简、移动或重命名仓库中的原始资料、压缩包、镜像、PDF、SDK、Sample 和硬件资料。
- 新代码默认放在 `work/` 下的新目录，或用户明确指定的工作目录中。
- 优先复制已有 Sample 的结构后修改，不要直接破坏原始 SDK 或 Sample。
- `Hi3403V100`、`SD3403V100`、`SS928V100` 在本项目中通常指同一平台的不同命名；判断具体兼容性时仍以文件、SDK 版本和硬件资料为准。
- 当前板端系统优先按 `Ubuntu 22.04` / Linux 用户态环境处理。
- 先查本地资料；本地没有再查在线资料。高频入口见 `docs/agent/repository-map.md`。
- 涉及板端网络、SSH、启动项、服务、pinmux、驱动或硬件状态修改时，先做只读探测；未经用户明确要求，不做破坏性或持久化修改。

## 2. 执行环境

每条命令必须明确属于且只属于一个目标：

- `LOCAL-WIN`：Windows 主机，使用 PowerShell 和 Windows 路径，例如 `C:\Users\ASUS\Desktop\ss928`。
- `BOARD-LINUX`：远程 SS928 板端，只能通过 SSH/SFTP/SCP 操作，使用 Linux 路径。
- `WSL-BUILD`：仅用于用户明确要求的 Linux 构建或交叉编译，从 PowerShell 显式调用 WSL。

规则：

- 默认环境是 `LOCAL-WIN`，不是 WSL。
- 不要在 Windows 本地终端直接运行裸 Linux 命令。
- 不要因为 WSL 和板端都是 Linux 就在 WSL 中代替板端操作。
- 不要在一个未标注的命令块中混用不同目标的命令和路径。
- 向用户展示命令、切换目标或执行板端操作时，明确标注执行目标。
- 命令失败后，先确认目标环境和路径是否正确，再诊断软件问题。
- 更完整规则见 `docs/agent/execution-environments.md`。

## 3. Skill 预检

开始项目、调试或阶段性任务前，先检查当前可用 skills 和仓库 `skills/` 目录。任务命中 skill 的描述或以下触发场景时，必须先完整阅读对应 `SKILL.md`：

- 直接连接、上传、运行、停止或调试 SS928 板端：`skills/ss928-direct-board-debug/SKILL.md`
- MAX98357 / 40Pin I2S 音频播放：`skills/ss928-max98357-audio-playback/SKILL.md`
- MT5710 / MT571X 5G RedCap 验证：`skills/ss928-mt5710-5g-validation/SKILL.md`
- 微信小程序开发、调试、预览或发布：`skills/miniprogram-development/SKILL.md`
- 原生 JavaScript 微信小程序实现：`skills/wechat-miniprogram-native/SKILL.md`
- 微信小程序云开发（CloudBase / 云函数 / 云数据库 / 云存储 / IoT）：必须调用 `skills/wechat-cloudbase-iot/SKILL.md`
- 智能安全书包小程序页面读取 CloudBase 最新状态、GNSS 轨迹或告警历史：必须调用 `skills/smartbag-cloud-read/SKILL.md`
- SS928 检测模块、传感器或模拟器向 CloudBase 上传状态、轨迹点或跌倒/碰撞告警：必须调用 `skills/smartbag-cloud-upload/SKILL.md`

若用户点名某个 skill，优先使用它。新建或更新 skill 后，同步更新 `docs/agent/skills-routing.md`。详细触发条件见该文件。

## 4. 按任务加载知识

只读取与当前任务直接相关的文档：

| 任务类型 | 必读文档 |
|---|---|
| 查资料入口、SDK 目录、版本或在线仓库副本 | `docs/agent/repository-map.md` |
| MPP、VI、VPSS、VO、VENC、RTSP、摄像头视频链路 | `docs/agent/mpp-camera-video.md` |
| Sensor、IMX347、转接板、I2C、lane、Sensor 时钟、MIPI 屏 | `docs/agent/camera-sensor-hardware.md` |
| GPIO、I2C、PWM、UART、ADC、温度、蓝牙、Wi-Fi、5G 外设 | `docs/agent/peripherals-and-40pin.md` |
| 固件、系统、内核、uboot、rootfs、镜像、SDK 补丁和构建 | `docs/agent/firmware-system-build.md` |
| OpenCV、Qt、YOLO、Python 调 MPP、ModelZoo | `docs/agent/advanced-resources.md` |
| 不清楚从哪里搜索或选哪个 Sample | `docs/agent/search-and-template-guide.md` |
| 复用已经验证的历史方案或避免旧坑 | `docs/agent/history/README.md`，然后只读对应项目文件 |
| 账号、默认 IP 或本地连接凭据 | `.local/device-access.md`；不得提交到 Git |

不要默认读取 `docs/agent/archive/AGENTS.original.md`。它只用于审计、恢复或确认迁移是否遗漏。

## 5. 资料和实现优先级

发生冲突时，默认优先级为：

1. 用户当前明确说明的实物、系统、接线和任务要求；
2. 当前板端 `Ubuntu 22.04` 的实际探测结果；
3. 本地适配 SDK `V2.0.2.2` 与配套本地 MPP Sample；
4. 本地更新版 `V2.0.2.3` Sample 和 Pegasus 等在线仓库副本；
5. 外部网页、论坛或通用经验。

摄像头、媒体链路和海思 API 代码优先从本地 Sample 复制或裁剪，不要凭空编写整条媒体管线。硬件引脚和 pinmux 必须查本地表格、资料或已验证记录，不要仅凭记忆。

## 6. 工作记录维护

- 完成独立项目或阶段性调试后，在 `docs/agent/history/` 新建或更新对应文件，并更新 `docs/agent/history/README.md`。
- 记录目标、最终方案、关键文件、验证结果、未验证项、踩坑和禁止重复的错误路线。
- 不要再把完整项目总结追加到根 `AGENTS.md`。
- 可复用且有明确触发条件的流程应沉淀为 skill，并在 `docs/agent/skills-routing.md` 登记。

## 7. 保密和本地文件

- 连接凭据保存在 `.local/device-access.md`，该目录必须保持 Git 忽略状态。
- 不要把密码复制进源码、公开文档、日志或提交记录。
- 若 `.local/device-access.md` 不存在，不要猜密码，向用户获取或使用其当前会话中明确提供的信息。
