# 外设 Sample、Driver 与 40Pin

> 涉及 GPIO、I2C、PWM、UART、ADC、温度、蓝牙、Wi-Fi、星闪或 5G 外设时读取。

## 外设代码怎么找

外设应用例程在 `HiEuler_PI_Peripherals_Sample-master`：

- `i2c_oled`：OLED 应用层控制。
- `mcu_tool`：控制海鸥派底板 MCU。
- `pwm`：PWM 舵机例程。
- `uart`：UART/RS485 读写例程。

外设驱动在 `HiEuler_PI_Peripherals_Driver-master`：

- `gpio`：内核态 GPIO 控制，默认示例是 40Pin Pin13/GPIO2_1。
- `hi_adc`：ADC 驱动，40Pin ADC 是 `LSADC_CH3`。
- `i2c_soft`：GPIO 模拟 I2C，四路 Sensor 时 I2C 不够用可参考。
- `oled`：OLED I2C 驱动。
- `RM500U`：移远 5G PCIe 模块驱动。
- `Tsensor`：SS928V100 芯片温度传感器，加载后 `cat /proc/Tsensor`。
- `ws73_sdk_linux_WS73_1.10.111`：WS73 星闪/蓝牙/Wi-Fi 三合一模块驱动。

40Pin 关键引脚来自 `预留IO引脚复用图.xlsx`：

- Pin3：`I2C0_SDA` / `GPIO11_4`，寄存器 `0x102F013c`。
- Pin5：`I2C0_SCL` / `GPIO11_5`，寄存器 `0x102F0140`。
- Pin11：`LSADC_CH3` / `GPIO10_0` / `PCIE_RST_N`，寄存器 `0x102F00FC`。
- Pin13：`GPIO2_1` / `USB_0_PWREN`，寄存器 `0x10230044`。
- Pin32：`PWM0_OUT1_0_P` / `GPIO17_0`，寄存器 `0x102F01EC`。PWM 例程使用 `bspmm 0x102f01ec 0x1201`。
- UART/RS485 例程提到 UART3 复用：`bspmm 0x102f012c 0x1201`、`bspmm 0x102f0130 0x1201`。
