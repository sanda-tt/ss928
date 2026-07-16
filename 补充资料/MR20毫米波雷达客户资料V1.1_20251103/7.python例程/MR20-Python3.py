#!/usr/bin/env python
# -*- coding:UTF-8 -*-
from __future__ import print_function
import serial

# 定义一个函数来解析数据包
def parse_data(data):
    byte3 = data[1]
    byte2 = data[0]
    print("\n")
    if byte3 == 0x06 and byte2 == 0x0b:
        byte4 = data[2]
        byte5 = data[3]
        byte6 = data[4]
        byte7 = data[5]
        byte8 = data[6]
        byte9 = data[7]
        byte10 = data[8]
        
        y_dist = (byte5 * 32 + (byte6 >> 3)) * 0.1 - 500
        x_dist = (((byte6 & 0x07) * 256 + byte7) * 0.1 - 102.3)
        y_vel = ((byte9 & 0x3f) * 8 + (byte10 >> 5)) * 0.25 - 64
        x_vel = ((byte8 << 2) + (byte9 >> 6)) * 0.25 - 128
        
        print("目标ID:", byte4, end="\t")
        print("y方向距离:", y_dist, end="\t")
        print("x方向距离:", x_dist, end="\t")
        print("y方向速度:", y_vel, end="\t")
        print("x方向速度:", x_vel, end="\t")
    elif byte3 == 0x06 and byte2 == 0x0a:
        idnum = data[2]
        print("*******************************")
        print("识别目标数量:", idnum, end="\t")

# 主程序开始
if __name__ == '__main__':
    last_angle = 0  # 初始化上一个角度
    # ser = serial.Serial('/dev/wheeltec_mmwave_radar', 115200)    # ubuntu，如果未修改串口别名，可通过 ll /dev 查看雷达具体端口再进行更改
    ser = serial.Serial("COM3", 115200, timeout=5)          # window 通过设备管理器查看串口号
    # 循环读取数据
    while True:
        try:
            data = ser.read(1)  # 读取1个字节的数据
            if data[0] == 0xAA:
                data = ser.read(1)  # 读取1个字节的数据
                if data[0] == 0xAA:
                    data = ser.read(12)
                    # 解析和打印数据包
                    if data[10] == 0x55 and data[11] == 0x55:
                        parse_data(data)
        except Exception as e:
            pass  # 忽略异常
