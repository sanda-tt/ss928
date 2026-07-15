#!/bin/bash
# Copyright (c) 2026 HiSilicon (Shanghai) Technologies Co., Ltd.
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

#
# Pre-build setup script
# Usage: Run from pegasus root directory after `repo sync -c`
#   ./os/OpenHarmony/manifest/prebuilts_setup.sh
#
# This script performs:
#   1. Fix system_util.py bug
#   2. Fix patch_process.py to add --forward flag
#   3. Copy platform/ss928v100_clang to sdk_linux
#   4. Download mbedtls v2.16.10
#   5. Download arm-trusted-firmware v2.2
#   6. Run build/prebuilts_download.sh
#   7. Checkout kernel_linux_patches/prebuilts via sparse-checkout
#   8. Checkout hisilicon/common/platform via sparse-checkout
#   9. Clean up sparse-checkout directories
#   10. Configure SDK toolchain environment (clang)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# ==============================================================================
# Configuration: URLs and Paths
# ==============================================================================

# Patch targets
SYSTEM_UTIL_FILE="$ROOT_DIR/os/OpenHarmony/build/hb/util/system_util.py"
PATCH_PROCESS_FILE="$ROOT_DIR/os/OpenHarmony/build/hb/util/prebuild/patch_process.py"

# Platform SDK copy
PLATFORM_SDK_SRC="$ROOT_DIR/platform/ss928v100_clang"
PLATFORM_SDK_DST="$ROOT_DIR/os/OpenHarmony/device/soc/hisilicon/hi3403v100/sdk_linux"

# mbedtls download
MBEDTLS_URL="https://github.com/ARMmbed/mbedtls/archive/refs/tags/v2.16.10.tar.gz"
MBEDTLS_DST_DIR="$ROOT_DIR/os/OpenHarmony/device/soc/hisilicon/hi3403v100/sdk_linux/open_source/mbedtls"
MBEDTLS_DST_FILE="$MBEDTLS_DST_DIR/mbedtls-2.16.10.tar.gz"

# arm-trusted-firmware download
ATF_URL="https://github.com/ARM-software/arm-trusted-firmware/archive/v2.2.tar.gz"
ATF_DST_DIR="$ROOT_DIR/os/OpenHarmony/device/soc/hisilicon/hi3403v100/sdk_linux/open_source/trusted-firmware-a"
ATF_DST_FILE="$ATF_DST_DIR/arm-trusted-firmware-2.2.tar.gz"

# Prebuilts download script
PREBUILTS_DOWNLOAD_SCRIPT="$ROOT_DIR/os/OpenHarmony/build/prebuilts_download.sh"

# Clang toolchain
CLANG_TOOLCHAIN_DIR="$ROOT_DIR/os/OpenHarmony/prebuilts/clang/ohos/linux-x86_64/llvm/bin"

# Kernel patches prebuilts (sparse-checkout)
KERNEL_PATCHES_REPO_URL="https://gitcode.com/openharmony/kernel_linux_patches.git"
KERNEL_PATCHES_DIR="$ROOT_DIR/os/OpenHarmony/kernel/linux/patches"
KERNEL_PATCHES_PREBUILTS_DIR="$KERNEL_PATCHES_DIR/linux-6.6/prebuilts"

# Hisilicon common platform (sparse-checkout from device_soc_hisilicon)
DEVICE_SOC_REPO_URL="https://gitcode.com/openharmony/device_soc_hisilicon.git"
HISILICON_COMMON_DIR="$ROOT_DIR/os/OpenHarmony/device/soc/hisilicon/common"
HISILICON_PLATFORM_DIR="$HISILICON_COMMON_DIR/platform"

# ==============================================================================
# Helper functions
# ==============================================================================

log_info() {
    echo "[prebuilts-setup] $1"
}

log_error() {
    echo "[prebuilts-setup] ERROR: $1" >&2
}

# ==============================================================================
# Tasks
# ==============================================================================

# Task 1: Fix system_util.py
fix_system_util() {
    log_info "Task 1: Fixing system_util.py..."
    
    if [ ! -f "$SYSTEM_UTIL_FILE" ]; then
        log_error "File not found: $SYSTEM_UTIL_FILE"
        return 1
    fi
    
    if grep -q 'while "" in cmd and type(cmd) == list:' "$SYSTEM_UTIL_FILE"; then
        log_info "Already patched, skipping."
        return 0
    fi
    
    if ! grep -q 'while "" in cmd:' "$SYSTEM_UTIL_FILE"; then
        log_error "Expected content not found in system_util.py"
        return 1
    fi
    
    sed -i 's/while "" in cmd:/while "" in cmd and type(cmd) == list:/' "$SYSTEM_UTIL_FILE"
    log_info "Successfully patched system_util.py"
}

# Task 2: Fix patch_process.py
fix_patch_process() {
    log_info "Task 2: Fixing patch_process.py..."
    
    if [ ! -f "$PATCH_PROCESS_FILE" ]; then
        log_error "File not found: $PATCH_PROCESS_FILE"
        return 1
    fi
    
    if grep -q "patch -p1 --forward <" "$PATCH_PROCESS_FILE"; then
        log_info "Already patched, skipping."
        return 0
    fi
    
    if ! grep -q "patch -p1 <" "$PATCH_PROCESS_FILE"; then
        log_error "Expected content not found in patch_process.py"
        return 1
    fi
    
    sed -i "s/cmd = f'patch -p1 < {patch_path}'/cmd = f'patch -p1 --forward < {patch_path}'/" "$PATCH_PROCESS_FILE"
    log_info "Successfully patched patch_process.py"
}

# Task 3: Copy platform SDK
copy_platform_sdk() {
    log_info "Task 3: Copying platform SDK..."
    
    if [ ! -d "$PLATFORM_SDK_SRC" ]; then
        log_error "Source directory not found: $PLATFORM_SDK_SRC"
        return 1
    fi
    
    mkdir -p "$PLATFORM_SDK_DST"
    
    for item in "$PLATFORM_SDK_SRC"/*; do
        local name="$(basename "$item")"
        if [ -d "$item" ]; then
            rm -rf "$PLATFORM_SDK_DST/$name"
            cp -a "$item" "$PLATFORM_SDK_DST/$name"
            log_info "Copied directory: $name"
        else
            cp -a "$item" "$PLATFORM_SDK_DST/$name"
            log_info "Copied file: $name"
        fi
    done
    
    log_info "Successfully copied platform SDK"
}

# Task 4: Download mbedtls
download_mbedtls() {
    log_info "Task 4: Downloading mbedtls v2.16.10..."
    
    mkdir -p "$MBEDTLS_DST_DIR"
    
    if [ -f "$MBEDTLS_DST_FILE" ]; then
        log_info "File already exists, skipping."
        return 0
    fi
    
    log_info "URL: $MBEDTLS_URL"
    curl -L -o "$MBEDTLS_DST_FILE" "$MBEDTLS_URL"
    log_info "Successfully downloaded mbedtls"
}

# Task 5: Download arm-trusted-firmware
download_trusted_firmware() {
    log_info "Task 5: Downloading arm-trusted-firmware v2.2..."
    
    mkdir -p "$ATF_DST_DIR"
    
    if [ -f "$ATF_DST_FILE" ]; then
        log_info "File already exists, skipping."
        return 0
    fi
    
    log_info "URL: $ATF_URL"
    curl -L -o "$ATF_DST_FILE" "$ATF_URL"
    log_info "Successfully downloaded arm-trusted-firmware"
}

# Task 6: Run prebuilts_download.sh
run_prebuilts_download() {
    log_info "Task 6: Running prebuilts_download.sh..."
    
    if [ ! -f "$PREBUILTS_DOWNLOAD_SCRIPT" ]; then
        log_error "File not found: $PREBUILTS_DOWNLOAD_SCRIPT"
        return 1
    fi
    
    chmod +x "$PREBUILTS_DOWNLOAD_SCRIPT"
    bash "$PREBUILTS_DOWNLOAD_SCRIPT"
    log_info "Successfully ran prebuilts_download.sh"
}

# Task 7: Checkout kernel_linux_patches/prebuilts via sparse-checkout
download_kernel_patches_prebuilts() {
    log_info "Task 7: Checking out kernel_linux_patches/prebuilts..."
    
    # Skip if prebuilts already exists
    if [ -d "$KERNEL_PATCHES_PREBUILTS_DIR" ]; then
        log_info "Prebuilts directory already exists, skipping."
        return 0
    fi
    
    # Clone to temp directory with sparse-checkout
    local TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    git init
    git remote add origin "$KERNEL_PATCHES_REPO_URL"
    
    # Enable sparse checkout and configure to only fetch linux-6.6/prebuilts
    git sparse-checkout init --cone
    git sparse-checkout set linux-6.6/prebuilts
    
    # Fetch only the linux-6.6/prebuilts directory
    git fetch --depth 1 origin OpenHarmony-5.1.0-Release
    git checkout -b local FETCH_HEAD
    
    # Copy to target location
    mkdir -p "$KERNEL_PATCHES_DIR/linux-6.6"
    cp -a "$TEMP_DIR/linux-6.6/prebuilts" "$KERNEL_PATCHES_PREBUILTS_DIR"
    
    # Cleanup temp directory
    rm -rf "$TEMP_DIR"
    cd "$ROOT_DIR"
    log_info "Successfully checked out kernel_linux_patches/prebuilts"
}

# Task 8: Checkout hisilicon/common/platform via sparse-checkout
download_hisilicon_platform() {
    log_info "Task 8: Checking out hisilicon/common/platform..."
    
    # Skip if platform already exists
    if [ -d "$HISILICON_PLATFORM_DIR" ]; then
        log_info "Platform directory already exists, skipping."
        return 0
    fi
    
    # Clone to temp directory with sparse-checkout
    local TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    git init
    git remote add origin "$DEVICE_SOC_REPO_URL"
    
    # Enable sparse checkout and configure to only fetch common/platform
    git sparse-checkout init --cone
    git sparse-checkout set common/platform
    
    # Fetch only the common/platform directory
    git fetch --depth 1 origin OpenHarmony-5.1.0-Release
    git checkout -b local FETCH_HEAD
    
    # Copy to target location
    mkdir -p "$HISILICON_COMMON_DIR"
    cp -a "$TEMP_DIR/common/platform" "$HISILICON_PLATFORM_DIR"
    
    # Cleanup temp directory
    rm -rf "$TEMP_DIR"
    cd "$ROOT_DIR"
    log_info "Successfully checked out hisilicon/common/platform"
}

# Task 9: Configure SDK toolchain environment
configure_toolchain_env() {
    log_info "Task 9: Configuring SDK toolchain environment..."
    
    if [ ! -d "$CLANG_TOOLCHAIN_DIR" ]; then
        log_error "Clang toolchain directory not found: $CLANG_TOOLCHAIN_DIR"
        return 1
    fi
    
    export PATH="$CLANG_TOOLCHAIN_DIR:$PATH"
    
    if command -v clang > /dev/null 2>&1; then
        log_info "Clang toolchain configured successfully: $(command -v clang)"
    else
        log_error "Failed to find clang in PATH"
        return 1
    fi
    
    log_info "Adding clang toolchain to ~/.bashrc for persistent configuration..."
    if ! grep -q "clang/ohos/linux-x86_64/llvm/bin" ~/.bashrc 2>/dev/null; then
        echo "" >> ~/.bashrc
        echo "# OpenHarmony Clang toolchain" >> ~/.bashrc
        echo "export PATH=$CLANG_TOOLCHAIN_DIR:\$PATH" >> ~/.bashrc
        log_info "Successfully added to ~/.bashrc"
    else
        log_info "Already configured in ~/.bashrc, skipping."
    fi
}

# Main
main() {
    log_info "============================================"
    log_info "Pre-build setup started"
    log_info "Root directory: $ROOT_DIR"
    log_info "============================================"
    
    local failed=0
    
    fix_system_util || failed=1
    fix_patch_process || failed=1
    copy_platform_sdk || failed=1
    download_mbedtls || failed=1
    download_trusted_firmware || failed=1
    run_prebuilts_download || failed=1
    download_kernel_patches_prebuilts || failed=1
    download_hisilicon_platform || failed=1
    configure_toolchain_env || failed=1
    
    log_info "============================================"
    if [ $failed -eq 0 ]; then
        log_info "Pre-build setup completed successfully"
        exit 0
    else
        log_error "Pre-build setup completed with errors"
        exit 1
    fi
}

main "$@"
