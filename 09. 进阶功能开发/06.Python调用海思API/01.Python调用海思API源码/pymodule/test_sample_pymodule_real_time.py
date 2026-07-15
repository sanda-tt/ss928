import sys
import cv2
import numpy as np
import time

import sample_pymodule

def test_sample_module():
    print("开始循环读取并处理帧... 按 Ctrl+C 退出")

    VPSS_GRP = 0
    VPSS_CHN = 1
    VO_LAYER = 0
    VO_CHN = 1

    while True:
        try:
            # 获取一帧
            frame = sample_pymodule.py_vpss_get_chn_frame(VPSS_GRP, VPSS_CHN)
        except Exception as e:
            print("获取帧失败:", e)
            continue

        try:
            # do something for frame
            # 转灰度
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

            # 二值化（阈值128）
            _, binary = cv2.threshold(gray, 128, 255, cv2.THRESH_BINARY)

            # 转回三通道
            binary_bgr = cv2.cvtColor(binary, cv2.COLOR_GRAY2BGR)

            try:
                sample_pymodule.py_vo_send_frame(VO_LAYER, VO_CHN, binary_bgr, VPSS_GRP, VPSS_CHN)
            except Exception as e:
                print("VO 输出失败:", e)

        finally:
            # 无论成功与否都要释放
            try:
                sample_pymodule.py_vpss_release_chn_frame(VPSS_GRP, VPSS_CHN)
            except Exception as e:
                print("释放帧失败:", e)

if __name__ == "__main__":
    print("py module test")
    try:
        test_sample_module()
    except KeyboardInterrupt:
        print("\n已手动中断")
