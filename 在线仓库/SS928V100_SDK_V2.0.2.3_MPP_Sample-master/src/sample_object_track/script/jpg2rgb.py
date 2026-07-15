# !/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
import io
import os
import glob
import argparse
import cv2
import numpy as np
from PIL import Image


def main(args):
    img_dir = args.img_dir
    out_dir = args.out_dir
    img_list = glob.glob(img_dir + "/*.jpg")
    img_list.sort()
    for img_file in img_list:
        img = cv2.imread(img_file)
        image_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        image_rgb = image_rgb.transpose(2, 0, 1)
        img_name = os.path.join(out_dir, os.path.basename(img_file))

        with open(img_name.replace("jpg", 'rgb'), 'wb') as rgb_file:  
            rgb_file.write(image_rgb.tobytes())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='jpg to rgb.')
    parser.add_argument('-i', '--img_dir', type=str, help='jpg Input Directory.', default='.')
    parser.add_argument('-o', '--out_dir', type=str, help='Directory for storing rgb output.', default='.')
    args = parser.parse_args()
    main(args)