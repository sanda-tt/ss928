#!/bin/bash

#Copyright (c) 2025 Nanjing Qinuo Information Technology Co., Ltd.

set -e

###########################  Global Paths ###########################
# PATCH Paths
PATCH_DIR_GCC="$(pwd)/patch/ss928v100_gcc"
PATCH_DIR_CLANG="$(pwd)/patch/ss928v100_clang"

# PEGASUS Paths
PEGASUS_DIR="$(pwd)/../../"

# PLATFORM Paths
PLATFORM_DIR_GCC="$(pwd)/../../platform/ss928v100_gcc"
PLATFORM_DIR_CLANG="$(pwd)/../../platform/ss928v100_clang"

# Sensor_libs Paths
SENSOR_TAR_GCC="$PATCH_DIR_GCC/sensor_libs.tar.xz"
SENSOR_TAR_CLANG="$PATCH_DIR_CLANG/sensor_libs.tar.xz"
SENSOR_DEST_GCC="$PLATFORM_DIR_GCC/smp/a55_linux/mpp/out/lib/"
SENSOR_DEST_CLANG="$PLATFORM_DIR_CLANG/smp/a55_linux/mpp/out/lib/"

# Linux Paths
LINUX_PATCH_GCC="$PATCH_DIR_GCC/linux/HiEulerPI_gcc_linux-6.6.y.patch"
LINUX_PATCH_CLANG="$PATCH_DIR_CLANG/linux/HiEulerPI_clang_linux-6.6.y.patch"
LINUX_DEST_GCC="$PLATFORM_DIR_GCC/open_source/linux/"
LINUX_DEST_CLANG="$PLATFORM_DIR_CLANG/open_source/linux/"
#####################################################################

usage(){
    echo "Usage: $0 [-all|-gcc|-clang|-h]"
    exit 0
}

MODE=${1--all}
[[ $MODE == -h ]] && usage

# Apply Patchs & Sensor Libs
apply_patches_and_extract(){
    local patch_dir=$1
    local plat_dir=$2
    local sensor_tar=$3
    local sensor_dest=$4
    local linux_patch=$5
    local linux_dest=$6

    [[ -d $patch_dir ]] || { echo "[WARN] Skip $patch_dir (not found)"; return; }
    [[ -d $plat_dir ]] || { echo "[ERROR] $plat_dir missing"; exit 1; }

    # Apply Patchs
    for patch in $(ls "$patch_dir"/*.patch 2>/dev/null | sort); do
        echo "Applying $(basename "$patch")"
        patch -d "$plat_dir" -p1 < "$patch"
    done

    # Sensor Libs
    if [[ -f $sensor_tar ]]; then
        tar -xf "$sensor_tar" -C "$sensor_dest" --strip-components=1
    fi

    # Linux Paths
    if [[ -f $linux_patch ]]; then
        cp -rf $linux_patch $linux_dest
    fi
}

case "$MODE" in
    -all)
        apply_patches_and_extract "$PATCH_DIR_GCC" "$PEGASUS_DIR" "$SENSOR_TAR_GCC" "$SENSOR_DEST_GCC" "$LINUX_PATCH_GCC" "$LINUX_DEST_GCC"
        apply_patches_and_extract "$PATCH_DIR_CLANG" "$PEGASUS_DIR" "$SENSOR_TAR_CLANG" "$SENSOR_DEST_CLANG" "$LINUX_PATCH_CLANG" "$LINUX_DEST_CLANG"
        ;;
    -gcc)
        apply_patches_and_extract "$PATCH_DIR_GCC" "$PEGASUS_DIR" "$SENSOR_TAR_GCC" "$SENSOR_DEST_GCC" "$LINUX_PATCH_GCC" "$LINUX_DEST_GCC"
        ;;
    -clang)
        apply_patches_and_extract "$PATCH_DIR_CLANG" "$PEGASUS_DIR" "$SENSOR_TAR_CLANG" "$SENSOR_DEST_CLANG"  "$LINUX_PATCH_CLANG" "$LINUX_DEST_CLANG"
        ;;
    *) usage ;;
esac

echo "All done."