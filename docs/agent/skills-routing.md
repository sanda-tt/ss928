# Skill 路由与维护

> 每个项目、调试或阶段性任务开始前用于 skill 预检。

## Skill 使用指南

每个项目、调试或阶段性任务开始前，必须先做 skill 预检，不要直接进入全仓搜索或写代码：

1. 先查看当前 Codex 可用 skills 和本仓库 `skills/` 目录。只要任务场景命中某个 skill 的 `description` 或本节触发场景，必须先完整阅读对应 `SKILL.md`，再开始操作。
2. 如果用户点名某个 skill，优先按该 skill 执行；如果当前环境不能使用它，要先说明原因，再选择最接近的替代流程。
3. 新建任何 skill 时，必须同步更新本节的“本项目已有 skill 触发场景”，写清 skill 名、什么时候用、关键入口或默认安全边界。不能只把 skill 放进目录而不登记。
4. 涉及板端、网络、蓝牙、5G、外设或长时间运行程序时，按对应 skill 的安全规则先做只读探测，再上传、启动、停止或改配置。
5. 项目结束后，如果沉淀了可复用流程，除了新增或更新 skill，也要在“项目工作总结”中记录目标、验证结果和踩坑。

本项目已有 skill 触发场景：

- `skills/ss928-direct-board-debug/SKILL.md`：只要需要直接连接 SS928/HiEuler Pi 板子调试，就必须先用。包括 SSH/SFTP、上传文件、运行板端命令、启动或停止程序、查看日志、BMI270/BLE/外设板端联调、确认 `root@192.168.1.168` 在线状态。默认先做只读探测，优先以太网 SSH，串口只作为 SSH 失败或看启动日志时的后备。
- `skills/ss928-max98357-audio-playback/SKILL.md`: use when the user provides MP3/WAV or other audio and wants playback through the 40Pin MAX98357 I2S amplifier. It prepares `audio_chn0.aac` plus 48k stereo PCM, sets Pin12/Pin38/Pin40 I2S pinmux, and reuses `/opt/sample/audio/sample_audio 2`; for board upload or SSH playback, pair it with `ss928-direct-board-debug` first.
- `skills/ss928-mt5710-5g-validation/SKILL.md`：测试或排查 TD Tech `MT5710/MT571X` 5G RedCap 模组时使用。包括 USB 枚举、`/dev/ttyUSB1` PCUI AT 指令、SIM/APN、蜂窝拨号、互联网连通、语音呼叫、GNSS/定位状态和 MT5710 专属问题。
- `skills/miniprogram-development/SKILL.md`：开发、修改、调试、预览、发布微信小程序时使用。包括页面、组件、`tabBar`、路由、图标资源、`project.config.json`、`appid`、真机预览、WeChat Developer Tools、`miniprogram-ci`、CloudBase/云开发相关流程。
- `skills/wechat-miniprogram-native/SKILL.md`：原生 JavaScript 微信小程序实现细节时使用，尤其是性能、包体、`setData` 数据路径、WXML/WXSS、原生组件兼容和避免 TypeScript/Taro/Uni-app 等跨端框架。
- `skills/smartbag-cloud-read/SKILL.md`：为智能安全书包微信小程序页面接入 CloudBase 已有数据时使用，包括展示最新设备状态、GNSS 历史轨迹、告警或跌倒记录。必须经由 `miniprogram/services/cloud-data-source.js` 调用已部署的 `smartbag-app-api`，不得在页面直接访问业务集合或破坏现有 BLE 实时链路；历史轨迹需按 `reportedAt` 正序映射为地图点，并以 MCP 的“上传记录 -> Event Function 返回 -> 页面”链路定位空数据问题。
- `skills/smartbag-cloud-upload/SKILL.md`：SS928 传感器、检测模块或模拟器需要上传状态、GNSS、姿态、风险、轨迹点及跌倒/碰撞告警到 CloudBase 时使用。必须复用已部署 `smartbag-device-ingest` 的既有遥测协议和 `X-Upload-Token`；HTTP 函数使用 `@cloudbase/node-sdk`，每个轨迹点必须带唯一 `requestId`，并在 MCP 审计后从 `cloudfunctions` 父目录部署。上传失败不得中断本地检测、BLE 或本地告警流程。
