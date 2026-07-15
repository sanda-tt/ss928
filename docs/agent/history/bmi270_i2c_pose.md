### work/bmi270_i2c_pose

- 目标：把新增补充资料里的 BMI270 六轴模块接到 SS928/HiEuler Pi，并在电脑端显示姿态数据。
- 资料结论：补充资料的 STM32 例程默认是 SPI，README 提醒姿态解算要按固定频率执行，示例为 200Hz；BMI270 数据手册确认芯片上电默认 I2C，CS/CSB 拉到 VDDIO 保持 I2C，SDO 决定 I2C 地址，SDO=GND 为 `0x68`，SDO=VDDIO 为 `0x69`。
- 推荐接法：VCC 接 40Pin Pin1/17 的 3.3V，GND 接任意 GND，SDA 接 Pin3 `I2C0_SDA`，SCL 接 Pin5 `I2C0_SCL`，CS 接 3.3V，SDO 默认接 GND，INT1 可接 Pin13 `GPIO2_1`，INT2 第一版先不接；如果需要第二路中断，可选空闲 GPIO 如 Pin15 `GPIO0_4`。
- 关键文件：`work/bmi270_i2c_pose/bmi270_i2c_pose.c` 是 Linux userspace I2C 读取和姿态解算程序；`viewer.html` 是电脑端 Web Serial 可视化；`pinmux_i2c0.sh` 设置 40Pin Pin3/Pin5 为 I2C0；`README.md` 写了接线、编译、运行和显示方式。
- 引脚复用：I2C0 复用值来自 SDK `smp/a55_linux/interdrv/sysconfig/pin_mux.c`，Pin3/Pin5 对应 `bspmm 0x102F013c 0x2031`、`bspmm 0x102F0140 0x2031`；INT1 可用 `bspmm 0x10230044 0x1200` 切到 GPIO2_1。
- 显示方案：板端程序输出 CSV，前三列固定为 `pitch,roll,yaw`，后面附带加速度、角速度、温度和时间；电脑可以直接用串口终端看文本，也可以用 Chrome/Edge 打开 `viewer.html` 选择串口后看 3D 立方体姿态。
- 验证结果：本机 Windows 没有 `gcc`，WSL 没有可用发行版，所以未在本机编译板端 C 程序；已确认 `BMI270_config.h` 本地存在，`viewer.html` 的脚本语法通过 Node 检查。内置浏览器因本地访问策略拦截 `file://`/localhost，未完成实际渲染截图。
- 踩坑：不要把 BMI270 的 VCC 或任何信号接 5V；第一版不需要中断脚，先轮询读通 I2C；如果通过 SSH 运行，CSV 会走 SSH 终端，不会自动进入浏览器串口可视化，要么在串口控制台运行，要么后续改 WebSocket/UDP。
