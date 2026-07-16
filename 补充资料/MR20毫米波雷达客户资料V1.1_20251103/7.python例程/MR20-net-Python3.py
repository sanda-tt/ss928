#!/usr/bin/env python
# -*- coding:UTF-8 -*-
from __future__ import print_function
import socket

# 定义一个函数来解析数据包
def parse_data(data):
    byte3 = data[3]
    byte2 = data[2]
    print("\n")
    if byte3 == 0x06 and byte2 == 0x0b:
        byte4 = data[4]
        byte5 = data[5]
        byte6 = data[6]
        byte7 = data[7]
        byte8 = data[8]
        byte9 = data[9]
        byte10 = data[10]
        
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
        idnum = data[4]
        print("*******************************")
        print("识别目标数量:", idnum, end="\t")

# 主程序开始
if __name__ == '__main__':
    last_angle = 0  # 初始化上一个角度
    # 创建 socket 对象
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # 目标雷达的 IP 和端口
    radar_ip = "192.168.1.200"
    radar_port = 2369

    # 本地电脑的 IP 和端口
    local_ip = "192.168.1.102"
    local_port = 2368
    try:
        # 绑定本地地址
        client_socket.bind((local_ip, local_port))
        # 连接到雷达
        # client_socket.connect((radar_ip, radar_port))
        print("已连接到雷达:", radar_ip, ":", radar_port)

        # 循环读取数据
        while True:
            try:
                data , addr = client_socket.recvfrom(14)
                if data[0] == 0xAA and data[1] == 0xAA:
                    if data[12] == 0x55 and data[13] == 0x55:
                        parse_data(data)
            except Exception as e:
                pass  # 忽略异常
    except Exception as e:
        pass  # 忽略异常