# SS928 BMI270 BLE 姿态小程序

这个小程序用于连接 SS928 板端的 `BMI270-Backpack` BLE 外设，实时展示 BMI270 姿态数据。

## 板端 BLE 参数

- 设备名：`BMI270-Backpack`
- Service UUID：`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX/write UUID：`6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX/notify UUID：`6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

板端每行发送一条 JSON，主要字段：

- `r/p/y`：roll、pitch、yaw，单位 deg。
- `a`：三轴加速度，单位 g。
- `w`：三轴角速度，单位 deg/s。
- `s/ag/gyr/st`：速度估算、加速度模长、角速度模长、静止状态。

## 使用方式

1. 在 SS928 上先确认 BMI270 接线为 I2C0：SDA -> Pin3，SCL -> Pin5，CS/CSB -> 3.3V，SDO -> GND。
2. 在 `work/linux_bmi270_backpack` 运行 `sh ./start_ss928_ble.sh`。
3. 用微信开发者工具打开本目录，真机预览。
4. 小程序首页点击“扫描设备”，连接 `BMI270-Backpack`。

注意：微信 BLE 需要真机验证，开发者工具模拟器不能替代真实手机蓝牙链路。
