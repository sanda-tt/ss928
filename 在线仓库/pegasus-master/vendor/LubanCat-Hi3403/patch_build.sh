#!/bin/bash -e
# Copyright (c) 2025 Dongguan EmbedFire Electronic Technology Co., Ltd.

CUR_DIR=$(pwd)
TOP_DIR=$(realpath "$CUR_DIR/../../")

if [ ! -d $TOP_DIR/platform/ss928v100_gcc ]; then
    echo "Check the current path, it must be run in vendor/LubanCat-Hi3403"
    exit 1
fi

# 复制文件
cp -vrf $CUR_DIR/files/* $TOP_DIR
# 复制补丁
cp -vrf $CUR_DIR/patch/* $TOP_DIR

# platform/ss928v100_gcc
cd $TOP_DIR/platform/ss928v100_gcc
patch -p1 < 0001-add-buildroot-rootfs-support.patch
patch -p1 < 0002-mpp-add-support-sony_imx415-sensor.patch
patch -p1 < 0003-mpp-drv-set-and-modify-mpp-for-lubancat.patch
