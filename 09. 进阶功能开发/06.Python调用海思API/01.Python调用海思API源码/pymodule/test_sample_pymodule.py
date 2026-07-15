import sys
import cv2
import numpy as np

import sample_pymodule

def test_sample_module():
    VPSS_GRP = 0
    VPSS_CHN = 1
    VO_LAYER = 0
    VO_CHN = 1

    frame = sample_pymodule.py_vpss_get_chn_frame(VPSS_GRP, VPSS_CHN)
    print("frame.shape =", frame.shape, "dtype =", frame.dtype)

    # do something for frame

    # 转灰度
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # 二值化（阈值128）
    _, binary = cv2.threshold(gray, 128, 255, cv2.THRESH_BINARY)

    # 转回三通道
    binary_bgr = cv2.cvtColor(binary, cv2.COLOR_GRAY2BGR)

    cv2.imwrite("./output/output_frame.jpg", frame)
    ret = sample_pymodule.py_vo_send_frame(VO_LAYER, VO_CHN, binary_bgr, VPSS_GRP, VPSS_CHN)
    print("py_vo_send_frame ret =", ret)

    sample_pymodule.py_vpss_release_chn_frame(VPSS_GRP, VPSS_CHN)

if __name__ == "__main__":
    print("py module test")
    test_sample_module()
