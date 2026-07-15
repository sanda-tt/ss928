# !/bin/bash

#################################################################################
# Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#################################################################################
ZSKS_PATH=$(pwd)
EBAINA_PATH=$(pwd)/../ebaina

# 回退到指定版本，ebaina更新patch后删除
############################################################
PEGASUS_PATH=$(pwd)/../../
CLANG_SDK_PATH=${PEGASUS_PATH}/platform/ss928v100_clang
GCC_SDK_PATH=${PEGASUS_PATH}/platform/ss928v100_gcc

############################################################

cd ${EBAINA_PATH}
chmod +x patch_build.sh
./patch_build.sh

cd ${CLANG_SDK_PATH}
patch -p1 < ${ZSKS_PATH}/patch/0001-support-USB-Camera-and-zsks-demo.patch

cd ${GCC_SDK_PATH}
patch -p1 < ${ZSKS_PATH}/patch/0001-support-USB-Camera-and-zsks-demo.patch


