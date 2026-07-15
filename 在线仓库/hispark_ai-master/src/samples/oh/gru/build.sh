#!/bin/bash
# Copyright (c) 2025-2025 HiSilicon (Shanghai) Technologies Co., Ltd

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
set -e

CUR_DIR=$(cd $(dirname $0) && pwd -P)

function prepare_env()
{
    rm -rf $SDK_PATH/output
    sample_cmakelists_content=$(< "$SDK_PATH/application/samples/CMakeLists.txt")
    cfg_content=$(< "$SDK_PATH/build/config/target_config/ws63/config.py")
}

function restore_sdk()
{
    cd $CUR_DIR
    echo "$sample_cmakelists_content" > $SDK_PATH/application/samples/CMakeLists.txt
    echo "$cfg_content" > $SDK_PATH/build/config/target_config/ws63/config.py
}

function set_build_env()
{
    if [ -z "$SDK_PATH" ]; then
        echo "ERROR: env SDK_PATH is empty, please set SDK_PATH"
        exit
    else
        echo "SDK_PATH=$SDK_PATH"
    fi
    if [ -z "$ADAPTOR_PATH" ]; then
        echo "ERROR: env ADAPTOR_PATH is empty, please set ADAPTOR_PATH"
        exit
    else
        echo "ADAPTOR_PATH=$ADAPTOR_PATH"
    fi
    if [ ! -f "$SDK_PATH/middleware/utils/ai_mcu/lib/libmicro_runtime.a" ]; then
        echo "ERROR: please use Converter_lite tools to generate lib (libmicro_runtime.a) and copy to \
${SDK_PATH}/middleware/utils/ai_mcu/lib"
        exit
    fi
    if [ ! -f "$SDK_PATH/middleware/utils/ai_mcu/lib/libnet.a" ]; then
        echo "ERROR: please use Converter_lite tools to generate lib (libnet.a) and copy to \
${SDK_PATH}/middleware/utils/ai_mcu/lib"
        exit
    fi
    export LANG="C"
    if [ ! -d "$SDK_PATH/middleware/utils/ai_mcu/adaptor" ]; then
        mkdir -p $SDK_PATH/middleware/utils/ai_mcu
        cp -rf $ADAPTOR_PATH/adaptor $SDK_PATH/middleware/utils/ai_mcu
    fi
    if [ ! -f "$SDK_PATH/include/middleware/utils/ai.h" ]; then
        cp -rf $ADAPTOR_PATH/include/ai.h $SDK_PATH/include/middleware/utils
    fi
}

function build_cfbb()
{
    echo CUR_DIR=$CUR_DIR
    # Replace
    export ENABLE_AI_CUSTOM_SAMPLE=y
    if ! grep -q "\$ENV{ENABLE_AI_CUSTOM_SAMPLE}" "$SDK_PATH/application/samples/CMakeLists.txt"; then
        sed -i '/COMPONENT_NAME/a\\nset(CONFIG_ENABLE_AI_CUSTOM_SAMPLE "$ENV{ENABLE_AI_CUSTOM_SAMPLE}")' $SDK_PATH/application/samples/CMakeLists.txt
    fi
    if ! grep -q "if(DEFINED CONFIG_ENABLE_AI_CUSTOM_SAMPLE)" "$SDK_PATH/application/samples/CMakeLists.txt"; then
        sed -i "/add_subdirectory_if_exist(custom)/i\if(DEFINED CONFIG_ENABLE_AI_CUSTOM_SAMPLE)\n\ \ add_subdirectory(\n\ \ \ \ ${CUR_DIR}\n\ \ \ \ \$\{CMAKE_CURRENT_BINARY_DIR\}/gru_build\n\ \ )\nendif()\n" $SDK_PATH/application/samples/CMakeLists.txt
    fi
    if ! grep -q "ai_adaptor_cpu" "$SDK_PATH/build/config/target_config/ws63/config.py"; then
        config_content=$(< "$SDK_PATH/build/config/target_config/ws63/config.py")
        config_content2=$(python -c "import json; $config_content; target['ws63-liteos-app']['ram_component'].append('ai_adaptor_cpu'); print(target)")
        echo "" > $SDK_PATH/build/config/target_config/ws63/config.py
        echo "target = $config_content2" >> $SDK_PATH/build/config/target_config/ws63/config.py
        echo "target_copy = {}" >> $SDK_PATH/build/config/target_config/ws63/config.py
        echo "target_group = {}" >> $SDK_PATH/build/config/target_config/ws63/config.py
    fi
    if ! grep -q "ai_mcu/adaptor/cpu" "$SDK_PATH/middleware/utils/CMakeLists.txt"; then
        echo "add_subdirectory_if_exist(ai_mcu/adaptor/cpu)" >> $SDK_PATH/middleware/utils/CMakeLists.txt
    fi
    cd $SDK_PATH
    if [ -f "$SDK_PATH/application/wb02_3.mk" ]; then
        ./build.py -c ws63-flashboot
    fi
    ./build.py -c ws63-liteos-app

    # Copy Output fwpkg
    mkdir -p $CUR_DIR/output
    cp $SDK_PATH/output/ws63/fwpkg/ws63-liteos-app/ws63-liteos-app_all.fwpkg $CUR_DIR/output/ws63-ai-liteos-sample.fwpkg
}

set_build_env

prepare_env

build_cfbb

restore_sdk
