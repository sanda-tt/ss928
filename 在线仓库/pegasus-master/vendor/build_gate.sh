#!/bin/bash

TARGET=$1
DIR=$2
SOC=$3
DEF=$4

#获取软件包&&打patch
function set_env()
{
    if [ ! -f /home/build/executed/set_env_executed ]; then
        echo "not set_env, start set_env!"
        cd /home/build/pegasus
        git submodule init
        git submodule update platform/ss928v100_clang platform/ss928v100_gcc
        
        cp /home/build/package/linux-6.6.86.tar.gz /home/build/pegasus/platform/ss928v100_clang/open_source/linux
        cp /home/build/package/mbedtls-2.16.10.tar.gz /home/build/pegasus/platform/ss928v100_clang/open_source/mbedtls
        cp /home/build/package/arm-trusted-firmware-2.2.tar.gz /home/build/pegasus/platform/ss928v100_clang/open_source/trusted-firmware-a
        cp /home/build/package/u-boot-2020.01.tar.bz2 /home/build/pegasus/platform/ss928v100_clang/open_source/u-boot

        cp /home/build/package/linux-6.6.86.tar.gz /home/build/pegasus/platform/ss928v100_gcc/open_source/linux
        cp /home/build/package/mbedtls-2.16.10.tar.gz /home/build/pegasus/platform/ss928v100_gcc/open_source/mbedtls
        cp /home/build/package/arm-trusted-firmware-2.2.tar.gz /home/build/pegasus/platform/ss928v100_gcc/open_source/trusted-firmware-a
        cp /home/build/package/u-boot-2020.01.tar.bz2 /home/build/pegasus/platform/ss928v100_gcc/open_source/u-boot
        
        if [[ "$DIR" =~ (/home/build/pegasus/vendor/[^/]*) ]]; then
            TARGET_DIR="${BASH_REMATCH[1]}"
        fi
        if [ -f ${TARGET_DIR}/patch_build.sh ]; then
            echo "patch_build.sh exists, start patch!"
            cd ${TARGET_DIR}
            chmod +x patch_build.sh
            ./patch_build.sh
        else
            echo "patch_build.sh not exists, skip patch!"
        fi

        touch /home/build/executed/set_env_executed
    else
        echo "set_env has been executed, skip!"
    fi
}

#编译Hi3403V100 clang案例
function build_clang_demo()
{
    set_env
    export PATH=/home/build/llvm/bin:$PATH
    export SYSROOT_PATH=/home/build/sysroot
    export PATH="/home/build/openeuler_gcc_arm64le/bin:$PATH"

    cd ${DIR}
    make LLVM=1 clean
    make LLVM=1
}

#编译Hi3403V100 gcc案例
function build_gcc_demo()
{
    set_env
    export PATH="/home/build/openeuler_gcc_arm64le/bin:$PATH"
    
    cd ${DIR}
    make LLVM=0 clean
    make LLVM=0
}

#帮助信息
function generate_usage()
{
    echo "Usage:  $0 [-option]"
    echo "options:"
    echo "./build_gate.sh SOC DEF DIR TARGET"
}

#解析参数
function parse_arg()
{
    
    case $SOC in
        "Hi3403V100")
            case $DEF in
                "CLANG")
                    build_clang_demo;
                    ;;
                "GCC")
                    build_gcc_demo;
                    ;;
            esac
            exit_status=$?
            ;;
        *)
            generate_usage;
            ;;
    esac
}

#开始时间
start_time=$(date +%s.%N)

#主函数入口
parse_arg

#结束时间
end_time=$(date +%s.%N)
cost_time=$(echo "$end_time $start_time" | awk '{printf "%.6f", $1 - $2}')
echo "$TARGET takes $cost_time s"

if [ $exit_status -eq 0 ]; then
    echo "######### Build target:$TARGET success #########"
    echo "Finished: SUCCESS"
else
    echo "######### Build target:$TARGET failure ########"
    echo "Finished: FAILURE"
fi
