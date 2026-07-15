#!/bin/bash
#Copyright (c) 2025 Beijing Xunwei Electronics Co., Ltd
if [ ! -f /var/lib/resize2fs_done ]; then
    resize2fs /dev/mmcblk0p3  # 或你的分区
    touch /var/lib/resize2fs_done
fi

cd /ko
bash load_ss928v100_ubuntu -i

sleep 2
sample_gfbg 0 0 0 &
sleep 3
startxfce4 &

while true
do
	sleep 300
	killall xfce4-screensaver
done
