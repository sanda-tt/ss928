# SS928 Agent 知识库索引

该目录承接原根 `AGENTS.md` 中的详细知识。根文件只保留常驻规则和路由；这里的文档由 agent 根据任务类型按需读取。

## 使用原则

1. 先读仓库根目录 `AGENTS.md`。
2. 根据其中的任务路由，只打开当前任务需要的文档。
3. 涉及历史项目时，先打开 `history/README.md`，再打开一个或少数几个相关项目文件。
4. `archive/AGENTS.original.md` 是无损原始备份，日常任务不要加载。
5. 账号信息只放在根目录 `.local/device-access.md`，不得提交 Git。

## 文档目录

- `execution-environments.md`：Windows、本地板端和 WSL 三种执行目标的详细规则。
- `skills-routing.md`：项目 skills 的触发条件和维护规范。
- `repository-map.md`：高频资料入口、SDK 结构和本地在线仓库副本。
- `mpp-camera-video.md`：MPP Sample 结构、媒体管线和编译要点。
- `camera-sensor-hardware.md`：Sensor 类型、转接板、I2C、lane、时钟和当前实物配置。
- `peripherals-and-40pin.md`：外设 Sample/Driver、40Pin 关键引脚和 pinmux 资料。
- `firmware-system-build.md`：虚拟机目录、固件打包、镜像、SDK 补丁与系统构建。
- `advanced-resources.md`：OpenCV、Qt、YOLO、Python MPP、ModelZoo 与综合案例。
- `search-and-template-guide.md`：任务查找策略、搜索命令和最小 Sample 选择。
- `history/`：已经验证的项目结果、踩坑和未验证项。
- `archive/`：原始完整 `AGENTS.md` 与校验值，仅用于审计和恢复。

## 维护规则

新增知识时，优先更新最具体的分类文件。只有全仓库必须始终生效的规则才允许加入根 `AGENTS.md`。历史验证记录不应回填到根文件。
