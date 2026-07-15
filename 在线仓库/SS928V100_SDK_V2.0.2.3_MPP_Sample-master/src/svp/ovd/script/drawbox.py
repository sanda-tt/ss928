#!/usr/bin/env python
# coding: utf-8
# Copyright (C) Shenshu Technologies Co., Ltd. 2022-2022. All rights reserved.
import os
import sys
import getopt
import numpy as np
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont


def draw_box(img_name, box_txt):
    box_value = []
    with open(box_txt, "r") as f:
        for line in f.readlines():
            line = line.strip('/n')
            temp = line.split()
            box_value.append(temp)
    img_width = int(box_value[0][0])
    img_height = int(box_value[0][1])
    img = Image.open(img_name)
    img_ori_w, img_ori_h = img.size
    if img_width != img_ori_w or img_height != img_ori_h:
        img = img.resize((img_width, img_height))
    for i in range(1, len(box_value)):
        txt = "{}: {}".format(box_value[i][0], box_value[i][1])
        d = ImageDraw.ImageDraw(img)
        d.text((int(float(box_value[i][2])) + 2, int(float(box_value[i][3])) + 1), txt, fill='blue')
        d.rectangle(((int(float(box_value[i][2])), int(float(box_value[i][3]))),
            (int(float(box_value[i][4])), int(float(box_value[i][5])))),
            fill=None, outline='blue', width=3)
    img_name = "out_" + os.path.basename(img_name)
    img.save(img_name)
    print("image {} write success.".format(img_name))


def main(argv):
    try:
        opts, args = getopt.getopt(argv, "hi:t:", ["ifile=", "tfile="])
    except getopt.GetoptError:
        print("drawBox.py -i <image> -t <boxTxt>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == "-i":
            input_file = arg
        elif opt == "-t":
            box_txt = arg
        else:
            print("drawBox.py -i <image> -t <boxTxt>")
    draw_box(input_file, box_txt)

if __name__ == "__main__":
    main(sys.argv[1:])