# Sensor、转接板与摄像头硬件

> 涉及 Sensor 类型、MIPI、IMX347、转接板、I2C、lane、时钟或显示屏时读取。

## Sensor / 摄像头注意事项

Sensor 类型在 `src/Makefile.param` 里改：

- `SENSOR0_TYPE ?= SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT`
- `SENSOR1_TYPE ?= ...`
- `SENSOR2_TYPE ?= ...`
- `SENSOR3_TYPE ?= ...`

已列出的 Sensor 类型包括 `HY_S0603`、`OV_OS08A20`、`SONY_IMX347`、`OV_OS05A10`、`SONY_IMX485`、`OV_OS04A10`、`OV_OS08B10`、`SC450AI`、`SONY_IMX219` 等。

转接板和 I2C 对应关系来自 MPP Sample README：

- `EULER_1R2D V1.0`：sensor0 4lane 使用 I2C5；dToF 2lane 可在 J3/I2C4 或 J4/I2C5。
- `EULER_2R V1.0`：J3 是 sensor0 4lane/I2C5，J4 是 sensor1 4lane/I2C7。
- `EULER_4SEN V1.0`：四路 2lane，sensor0/I2C7，sensor1/I2C5，sensor2/I2C4，sensor3/I2C6。

Sensor 支持组合：

- IMX347：1R2D 支持 4lane sensor0；2R 支持 sensor0~1；4SEN 支持 2lane sensor0~3。
- OS04A10、OS08A20：1R2D/2R 支持 4lane；4SEN 不支持。
- SC450AI：1R2D/2R 支持 4lane；4SEN 支持 2lane sensor0~3。

当前用户实物连接：

- MIPI 显示屏是 `800x1280`。
- Sensor 转接板是 `EULER_4SEN V1.0`，也就是 4x2lane VIO。
- 目前只在 `sensor0` 接了一个 IMX347 摄像头；按 4SEN 资料，sensor0 对应 `I2C7`，Sensor 类型应优先用 `SONY_IMX347_2L_SLAVE_MIPI_2M_30FPS_12BIT`，lane divide 参考更新 sample 的四路 2lane 配置用 `LANE_DIVIDE_MODE_3`。

Sensor 时钟：

- 可改 `load_ss928v100` 脚本参数，例如 `./load_ss928v100 -i -sensor0 os08a20 -sensor1 os08a20 -sensor2 os08a20 -sensor3 os08a20`。
- 也可用寄存器：Sensor0 `bspmm 0x11018440 ...`，Sensor1 `0x11018460`，Sensor2 `0x11018480`，Sensor3 `0x110184A0`。
- IMX347 时钟配置值 `0x8001`，37.125MHz。
- OS04A10/OS08A20 时钟配置值 `0x4001`，24MHz。
- SC450AI 时钟配置值 `0xA001`，27MHz。

硬件资料位置：

- 摄像头模块：`07硬件资料/03.外设模块资料/07.摄像头模组`
- Sensor 转接板：`07硬件资料/03.外设模块资料/07.摄像头模组/05.Sensor转接板`
- dToF：`07硬件资料/03.外设模块资料/04.dToF`
- MIPI 显示屏：`07硬件资料/03.外设模块资料/05.MIPI显示屏`
