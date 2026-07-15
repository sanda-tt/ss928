#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# Copyright (C) Shenshu Technologies Co., Ltd. 2022-2022. All rights reserved.
import os
import sys
import math
import numpy as np
import cv2

W = 640
H = 640


def process(input_path):
    try:
        print(input_path)
        input_image = cv2.imread(input_path)
        size = input_image.shape
        width = size[1]
        height = size[0]

        r = min(W/width, H/height)
        neww, newh = (min(math.ceil(width * r), W), min(math.ceil(height * r), H))
        input_image = cv2.resize(input_image, (neww, newh))

        new_shape = [H, W]
        shape = size
        r = min(new_shape[0] / shape[0], new_shape[1] / shape[1])
        r = min(r, 1.0)
        new_unpad = int(round(shape[1]*r)), int(round(shape[0]*r))
        dw, dh = new_shape[1] - new_unpad[0], new_shape[0] - new_unpad[1]
        dw /= 2 # 2 means half
        dh /= 2 # 2 means half
        if shape[::-1] != new_unpad:
            input_image = cv2.resize(input_image, new_unpad, interpolation=cv2.INTER_LINEAR)
        top, bottom = int(round(dh - 0.1)), int(round(dh + 0.1)) # 0.1 for round
        left, right = int(round(dw - 0.1)), int(round(dw + 0.1)) # 0.1 for round

        input_image = cv2.copyMakeBorder(input_image, top, bottom, left, right,
            cv2.BORDER_CONSTANT, value=(114, 114, 114)) # 114 color

        # hwc
        output_img = os.path.splitext(os.path.basename(input_path))[0]
        output_name = os.path.join(os.getcwd(), '..', 'img', 'binfile', output_img + '_ovd.bin')
        output_img = os.path.join(os.getcwd(), '..', 'img', 'resizeimg', output_img + '.jpg')

        cv2.imwrite(output_img, input_image)
        img = np.array(input_image)
        # rgb to bgr
        img = img[:, :, ::-1]
        shape = img.shape
        img = img.astype("uint8")
        img = img.reshape([1] + list(shape))
        result = img.transpose([0, 3, 1, 2])
        result.tofile(output_name)
    except Exception as except_err:
        print(except_err)
        return 1
    else:
        return 0


if __name__ == "__main__":
    count_ok = 0
    count_ng = 0
    images = os.listdir(r'./')
    image_dir = os.path.realpath("./")
    for image_name in images:
        if not (image_name.lower().endswith((".bmp", ".dib", ".jpeg", ".jpg", ".jpe",
        ".png", ".pbm", ".pgm", ".ppm", ".sr", ".ras", ".tiff", ".tif"))):
            continue
        print("start to process image {}....".format(image_name))
        image_path = os.path.join(image_dir, image_name)
        ret = process(image_path)
        if ret == 0:
            print("process image {} successfully".format(image_name))
            count_ok = count_ok + 1
        elif ret == 1:
            print("failed to process image {}".format(image_name))
            count_ng = count_ng + 1
    print("{} images in total, {} images process successfully, {} images process failed"
          .format(count_ok + count_ng, count_ok, count_ng))
