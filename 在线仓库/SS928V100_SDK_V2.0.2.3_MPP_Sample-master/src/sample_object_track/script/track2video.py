# !/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
import os
import argparse
import cv2
from tqdm import tqdm


def main(args):
    # 修改为实际的sample工程路径
    image_folder = args.input_dir
    track_txt_path = args.result

    track_infos = []
    with open(track_txt_path, "r") as t_f:
        lines = t_f.readlines()
        lines.sort()
        for line in lines:
            data = line.strip().split()
            box_info = [float(val) for val in data[1:]]
            track_infos.append(box_info)

    # 获取文件夹中所有图像文件名
    images = [img for img in os.listdir(image_folder) if img.endswith(".jpg")]
    images.sort()
    first_name = images[0]
    frame = cv2.imread(os.path.join(image_folder, first_name))
    height, width, layers = frame.shape

    video = cv2.VideoWriter(args.out_dir,
                    cv2.VideoWriter_fourcc(*'DIVX'), 25, (width, height))

    for image, box_info in tqdm(zip(images, track_infos)):
        cur_frame = cv2.imread(os.path.join(image_folder, image))
        if box_info[4] > 0.0:
            cv2.rectangle(cur_frame, (int(box_info[0]), int(box_info[1])),
                (int(box_info[2]), int(box_info[3])), (0, 0, 255), 2)
            cv2.putText(cur_frame, str(box_info[4]), (int(box_info[0]), int(box_info[1]-20)), 
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
    
        video.write(cur_frame)

    cv2.destroyAllWindows()
    video.release()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Converting Trace Results to Videos.')
    parser.add_argument('-i', '--input_dir', type=str, help='jpg files for composite video.')
    parser.add_argument('-r', '--result', type=str, help='result.txt.')
    parser.add_argument('-o', '--out_dir', type=str, help='Path for storing the output video.', default='result.avi')
    args = parser.parse_args()
    main(args)
