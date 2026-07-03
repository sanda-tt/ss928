# SS928 雷达 UART 第一步测试

这个目录用于把觅感 MS60-3015S80M4 60GHz 雷达模块先接到 SS928/HiEuler Pi 上做最小验证：先不接入业务程序，只把雷达串口传出来的数据打印出来。

## 接线

优先用 40Pin 上的 UART4。它比较好接，固件里的引脚复用脚本也已经覆盖了这组引脚。

下面的接法默认接在 EULER_40PEXP 40Pin 扩展板外侧引脚。扩展板原理图里有 1.8V 到 3.3V 的电平转换；如果绕过扩展板、直接接 SS928 SoC 原始 UART4 管脚，UART4_RXD/UART4_TXD 是 1.8V IO，不能和 3.3V 雷达直连。

| 雷达模块引脚 | 方向 | SS928 40Pin 连接 |
| --- | --- | --- |
| 3.3V | 电源输入 | Pin1 或 Pin17, 3.3V |
| GND | 地 | 任意 GND, 如 Pin6/9/14/20/25/30/34/39 |
| TX | 雷达输出 | Pin10, UART4_RXD |
| RX | 雷达输入 | Pin8, UART4_TXD |
| OUT | 雷达 GPIO 输出 | Pin13, GPIO2_1, Linux GPIO 17 |

注意：

- TX/RX 要交叉接：雷达 TX 接板子 RX，雷达 RX 接板子 TX。
- 雷达 UART 是 3.3V TTL 电平，不要接到 RS485，也不要接 5V UART。
- OUT 只是一个简单检测输出脚，完整目标数据还是走 UART。

## 板端引脚复用

如果启动脚本没有自动配置 UART4，先在板端执行：

```sh
bspmm 0x102F0134 0x1201  # 40Pin Pin10 -> UART4_RXD
bspmm 0x102F0138 0x1201  # 40Pin Pin8  -> UART4_TXD
bspmm 0x10230044 0x1200  # 40Pin Pin13 -> GPIO2_1
```

## 在板端编译

```sh
cd /path/to/work/radar_uart_test
make
```

如果在交叉编译环境里编译：

```sh
make CC=aarch64-mix210-linux-gcc
```

## 运行

协议文档写的默认串口参数是 921600 bps，8 数据位，1 停止位，无校验。

```sh
sudo ./radar_uart_dump /dev/ttyAMA4 921600 --probe
```

输出含义：

- `RX ...` 是雷达串口收到的原始字节。
- `REPORT type=0x07` 是 BSD 盲区检测目标信息。
- `CI response` 表示 `--probe` 发送的软件/硬件版本查询命令收到了回复。

如果没有输出，先在雷达前移动目标再试。协议说明主动上报一般是检测到目标才输出。如果 `--probe` 也没有回复，重点检查 TX/RX 是否交叉、`/dev/ttyAMA4` 是否存在；如果之前用过 PC 工具改过参数，再试 `115200`。

## 可选：检查 OUT 脚

OUT 可以单独通过 GPIO2_1 读：

```sh
bspmm 0x10230044 0x1200
echo 17 | sudo tee /sys/class/gpio/export
echo in | sudo tee /sys/class/gpio/gpio17/direction
watch -n 0.1 cat /sys/class/gpio/gpio17/value
```
