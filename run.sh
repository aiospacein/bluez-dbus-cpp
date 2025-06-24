#!/bin/bash

clear
echo "[INFO] Setting up bluez-dbus-cpp build environment..."

set -e
trap 'echo "[ERROR] Script interrupted or failed"; exit 1' ERR

TARGET_USER="root"
TARGET_DIR="/usr/local"
APP_NAME="example"
OUT_BINARY="../build/bin/$APP_NAME"

# Detect build type
if [[ "$1" == "R" || "$1" == "r" ]]; then
    BUILD_TYPE="Release"
else
    BUILD_TYPE="Debug"
fi

# Rebuild flag
REBUILD_FLAG=false
if [[ "$2" == "rebuild" ]]; then
    echo "[INFO] Rebuild flag detected. Cleaning build directory..."
    REBUILD_FLAG=true
fi

# Check previous target
PREV_TARGET=""
if [[ -f "build/CMakeCache.txt" ]]; then
    if grep -q "TARGET_STM32:BOOL=ON" "build/CMakeCache.txt"; then
        PREV_TARGET="STM32"
    else
        PREV_TARGET="Linux"
    fi
fi

# Detect target IP over USB
get_target_ip() {
    echo "[INFO] Detecting STM32MP1 IP (USB)..."
    IP_CANDIDATES=$(ip a | grep -A 2 "enx" | grep inet | awk '{print $2}' | cut -d/ -f1)
    TARGET_IPv4=$(echo "$IP_CANDIDATES" | grep -oP '^\d+\.\d+\.\d+\.\d+')
    if [[ -z "$TARGET_IPv4" ]]; then
        echo "[ERROR] No IP found on USB interface. Is STM32MP1 connected?"
        exit 1
    fi
    TARGET_IP=$(echo "$TARGET_IPv4" | sed 's/\.[0-9]*$/.1/')
    echo "[INFO] Found target IP: $TARGET_IP"
}

# Select build target
echo "Choose build target:"
echo "1) STM32MP1 (cross-compilation)"
echo "2) Native Linux (host build)"
read -rp "Enter choice [1 for STM32MP1 or any other key for native]: " choice

if [[ "$choice" == "1" ]]; then
    CURRENT_TARGET="STM32"
else
    CURRENT_TARGET="Linux"
fi

# Clean build if needed
if [[ "$REBUILD_FLAG" == "true" || "$PREV_TARGET" != "$CURRENT_TARGET" && -n "$PREV_TARGET" ]]; then
    echo "[INFO] Cleaning build directory due to rebuild or target change..."
    rm -rf build
fi

mkdir -p build
cd build
sudo chown -R $USER:$USER .

# Optional protocol regeneration flag
if [[ "$REBUILD_FLAG" == "true" ]]; then
    CMAKE_FLAGS="-DFORCE_PROTOCOL_REGENERATION=ON"
else
    CMAKE_FLAGS=""
fi

# Configure & build
if [[ "$CURRENT_TARGET" == "STM32" ]]; then
    echo "[INFO] Building for STM32MP1..."
    SDK_ENV="/home/aiospace/STM32MPU_workspace/STM32MPU-Ecosystem-v6.0.0/Developer-Package/SDK/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi"
    if [[ ! -f "$SDK_ENV" ]]; then
        echo "[ERROR] SDK environment file not found!"
        exit 1
    fi
    source "$SDK_ENV"
    echo "[DEBUG] CC=$CC"
    cmake .. -DTARGET_STM32=ON -DBUILD_EXAMPLE=1 -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_FLAGS
else
    echo "[INFO] Building for native Linux..."
    cmake .. -DTARGET_STM32=OFF -DBUILD_EXAMPLE=1 -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_FLAGS
fi

cmake --build .

# Validate binary
if [[ ! -f "$OUT_BINARY" ]]; then
    echo "[ERROR] Build failed: $OUT_BINARY not found."
    exit 1
fi

# Run/deploy
if [[ "$CURRENT_TARGET" == "STM32" ]]; then
    get_target_ip

    echo "[INFO] Copying binary to target..."
    scp "$OUT_BINARY" "$TARGET_USER@$TARGET_IP:$TARGET_DIR"

    echo "[INFO] Setting execute permission..."
    ssh "$TARGET_USER@$TARGET_IP" "chmod +x $TARGET_DIR/$APP_NAME"

    echo "[INFO] Running app on target..."
    ssh "$TARGET_USER@$TARGET_IP" "cd $TARGET_DIR && ./$APP_NAME"
else
    echo "[INFO] Running $APP_NAME locally..."
    ./"$OUT_BINARY"
fi
