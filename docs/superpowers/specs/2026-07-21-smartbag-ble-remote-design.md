# SmartBag 蓝牙遥控测试页设计

## 目标

恢复 SmartBag 的蓝牙四级预警遥控测试，同时保留现有云端实时姿态页。

## 范围

- 新增 `miniprogram/pages/remote/` 原生小程序页面。
- 首页最底部的调试入口改为跳转 `/pages/remote/index`。
- 遥控页扫描并连接广播名为 `SS928-SmartBag` 的 Nordic UART Service 外设。
- 遥控页可发送 `AL L1` 至 `AL L4`、`AL R1` 至 `AL R4` 和 `AL CLEAR`。
- 页面展示蓝牙状态、设备名、最后发送命令和板端文本回复。
- 未连接时，预警与停止按钮不可用；扫描、连接、服务发现或写入失败时显示可理解的错误提示。

## 非目标

- 不修改 `pages/monitor/index` 的云端姿态展示。
- 不修改板端 SmartBag 服务、BLE UUID、预警协议、硬件或 CloudBase 数据。
- 不新增依赖或跨端框架。

## 页面与数据流

`pages/home/index` 的底部调试卡导航至 `pages/remote/index`。遥控页使用既有 NUS UUID：服务 UUID `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`、写入 RX UUID `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`、通知 TX UUID `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`。收到 TX 通知后，以换行重组成板端文本回复。

每个预警按钮通过 `data-command` 映射到一条 ASCII 指令。发送前页面必须已经完成连接、服务发现和 RX 特征识别。页面离开时停止扫描、取消通知并断开当前 BLE 连接，避免残留回调。

## 验证

- 单元测试验证首页调试入口的路由是 `/pages/remote/index`。
- 单元测试验证遥控页包含 8 条四级指令、停止指令、NUS UUID 和未连接保护。
- 使用 `node --check` 验证新增和改动的 JavaScript。
- 微信开发者工具可验证页面编译；BLE 扫描、连接、写入与板端回复必须通过真机预览验证。
