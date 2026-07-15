#!/bin/bash
make_patch() {
    # 检查参数是否正确
    if [ $# -ne 3 ]; then
        echo "Usage:$0 <dir_A> <dir_B> <patch_name>"
        return 1
    fi
    # 入参1为：OH原生代码仓绝对路径，如/pathto/OpenHarmony-v5.1.0-Release/OpenHarmony/kernel/linux/config
    # 入参2为：产品化特性基于OH原生代码仓定制化修改的绝对路径 ，如/pathto/sig/master/test/kernel/linux/config
    # 入参3为：git对比生成差异后的patch文件名称 ，如0001_kernel_linux_config.patch
    local old_dir="$1"
    local new_dir="$2"
    local patch_file="$3"
    local current_dir=$(pwd)
    local temp_dir=$(mktemp -d)
    cp -rf "$old_dir" "$temp_dir"/A
    cd "$temp_dir"/A || exit
    rm -rf .git
    # 初始化git仓库并提交
    git init > /dev/null
    git add --all
    git commit -m "Initial commit (A)" > /dev/null
    rm -rf *
    # 复制目录B到临时目录
    cp -rf "$new_dir"/* ./
    git add --all
    git commit -m "Update to B" > /dev/null
    # 生成补丁文件
    git diff HEAD^ HEAD > "$current_dir/$patch_file"
    # 返回补丁文件路径
    echo "Patch file generated: $current_dir/$patch_file"
    # 清理临时目录,若脚本清理不掉可手动命令清理
    cd ..
    rm -rf "$temp_dir"
}

make_patch $1 $2 $3

