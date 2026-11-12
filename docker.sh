#!/usr/bin/env bash
# Core/docker.sh — thin client на .ci/scripts/docker-lib.sh (CoreCI v0.2.0+).
set -euo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
[ -f "$APP_ROOT/.ci/scripts/docker-lib.sh" ] || {
    echo "ERROR: CoreCI submodule not initialized (need v0.2.0+).
  Run: git submodule update --init --recursive
       (cd .ci && git checkout v0.2.0)" >&2
    exit 1
}

APP_NAME="core"
APP_VERSION="$(tr -d '[:space:]' < "$APP_ROOT/VERSION")"

# Defaults из env (как раньше)
CORE_BASE_DEPS_LINUX_QT6="${CORE_BASE_DEPS_LINUX_QT6:-base-deps-linux-qt6:latest}"
CORE_BASE_DEPS_LINUX_QT5="${CORE_BASE_DEPS_LINUX_QT5:-base-deps-linux:latest}"
CORE_BASE_DEPS_WIN64_QT6="${CORE_BASE_DEPS_WIN64_QT6:-base-deps-win64-qt6:latest}"
CORE_BASE_DEPS_WIN_QT5="${CORE_BASE_DEPS_WIN_QT5:-base-deps-win:latest}"
CORE_BASE_DEPS_ARM64_QT6="${CORE_BASE_DEPS_ARM64_QT6:-base-deps-arm64-qt6:latest}"
CORE_BASE_DEPS_ARM64_QT5="${CORE_BASE_DEPS_ARM64_QT5:-base-deps-arm64:latest}"
CORE_BASE_DEPS_ARMV7_QT5="${CORE_BASE_DEPS_ARMV7_QT5:-base-deps-armv7:latest}"

declare -A PLATFORMS=(
    [linux-qt6]="dockerfile=Dockerfile.core;out_subdir=linux-qt6;pkg_suffix=linux-qt6;base_image=${CORE_BASE_DEPS_LINUX_QT6};buildx_platform=linux/amd64;qt_major=6;qt6_prefix=/opt/Qt/6.8.0/gcc_64;sdk_platform=linux-amd64"
    [linux-qt5]="dockerfile=Dockerfile.core;out_subdir=linux-qt5;pkg_suffix=linux-qt5;base_image=${CORE_BASE_DEPS_LINUX_QT5};buildx_platform=linux/amd64;qt_major=5;sdk_platform=linux-amd64"
    [win-qt6]="dockerfile=Dockerfile.core;out_subdir=win-qt6;pkg_suffix=win-qt6;base_image=${CORE_BASE_DEPS_WIN64_QT6};cmake_cmd=cmake;qt_major=6;qt6_prefix=/opt/Qt/6.8.0/mingw_64;toolchain=/opt/x86_64-w64-mingw32.cmake;sdk_platform=win64;mingw_triplet=x86_64-w64-mingw32;mingw_pei=pei-x86-64;mingw_gcc_suffix=seh-1"
    [win-qt5]="dockerfile=Dockerfile.core;out_subdir=win-qt5;pkg_suffix=win-qt5;base_image=${CORE_BASE_DEPS_WIN_QT5};cmake_cmd=i686-w64-mingw32-cmake;qt_major=5;sdk_platform=win32;mingw_triplet=i686-w64-mingw32;mingw_pei=pei-i386;mingw_gcc_suffix=dw2-1"
    [arm64-qt6]="dockerfile=Dockerfile.core;out_subdir=arm64-qt6;pkg_suffix=arm64-qt6;base_image=${CORE_BASE_DEPS_ARM64_QT6};buildx_platform=linux/arm64;qt_major=6;qt6_prefix=/opt/Qt/6.8.5/gcc_arm64;sdk_platform=linux-arm64"
    [arm64-qt5]="dockerfile=Dockerfile.core;out_subdir=arm64-qt5;pkg_suffix=arm64-qt5;base_image=${CORE_BASE_DEPS_ARM64_QT5};buildx_platform=linux/arm64;qt_major=5;sdk_platform=linux-arm64"
    [armv7-qt5]="dockerfile=Dockerfile.core;out_subdir=armv7-qt5;pkg_suffix=armv7-qt5;base_image=${CORE_BASE_DEPS_ARMV7_QT5};buildx_platform=linux/arm/v7;qt_major=5;sdk_platform=linux-armv7"
)

source "$APP_ROOT/.ci/scripts/docker-lib.sh"

# PLATFORMS_LIST — какой env ожидает lib; клиент не заморачивается
core_docker_main "$@"