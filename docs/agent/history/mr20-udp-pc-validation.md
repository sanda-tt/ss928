# MR20 UDP PC 端验证

## 目标

在 Windows 主机先验证 MR20 网口雷达的 UDP 接收、官方 `0x60B` 目标帧解析、控制台输出与 CSV 日志；未接入 SS928 板端。

## 环境与协议事实

- 官方资料目录：`补充资料/MR20毫米波雷达资料`。
- 默认雷达 IP/端口与主机监听端口可参考官方 `7.python例程/MR20-net-Python3.py`：雷达 `192.168.1.200:2369`，PC 监听 `2368`。
- UDP 数据为完整 14 字节帧：`AA AA`、小端 CAN ID、8 字节数据、`55 55`；目标通用帧 CAN ID 是 `0x60B`，线上字节为 `0B 06`。
- 坐标保持官方原始定义：`x` 是横向，`y` 是纵向。动态属性来自第 6 个数据字节的低 3 位：`stopped`、`oncoming`、`going`、`crossing`。

## 最终方案与关键文件

- `work/radar/drivers/mr20/mr20_udp.py`：UDP 绑定、可选来源 IP 过滤、接收、控制台输出和日志追加；默认写入 `work/radar/logs/radar.log`。
- `work/radar/drivers/mr20/mr20_parser.py`：严格验证帧头、帧尾、长度与 `0x60B`，按官方公式解析 ID、横/纵距离、横/纵速度和动态属性。
- `work/radar/drivers/mr20/radar_target.py`：数据模型及指定控制台/CSV 格式。
- `work/radar/drivers/mr20/test_mr20.py`：离线协议和 UDP 回环验证入口。

## 验证结果

在 Windows 本机运行 `python -X utf8 work\\radar\\drivers\\mr20\\test_mr20.py` 通过 4 项测试：官方协议第 9.4 节的 `0x60B` 目标样本、非法帧拒绝、目标输出/CSV 字段，以及 UDP 回环接收并写入日志。

## 未验证项

尚未连接实体 MR20；因此主机网卡配置、防火墙放行和真实雷达 UDP 数据流需在接入设备后现场验证。

## 踩坑与禁止路线

- 不要复用 ROS 示例的坐标旋转；它面向 ROS 右手坐标系，会改变 PC 原始协议验证的 x/y 含义。
- 不要把 14 字节帧中的 `0x60B` 当作大端；网口官方示例使用低字节在前的 `0B 06`。
- 不要直接记录未规整的浮点计算结果；按协议分辨率规整后再输出，避免 CSV 出现 `3.000000000000014`。
