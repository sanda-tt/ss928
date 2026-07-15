#!/bin/sh

patch_dir=${1}
echo "${patch_dir}: ${1}"
echo "Current directory $(pwd)"
echo "patching ss928v100 begin..."
patch -p1 < ${patch_dir}/oh_ss928.patch
patch -p1 < ${patch_dir}/oh_feature.patch
patch -p1 < ${patch_dir}/0001-support-eulerpi-uvc-and-ethernet.patch
echo "patching ss928v100 finished..."
