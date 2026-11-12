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

PLATFORMS_LIST="${PLATFORMS:-${PLATFORMS_LIST:-linux-qt6}}"

declare -A PLATFORMS=(
    [linux-qt6]="linux-qt6|linux-qt6|Dockerfile.core|${CORE_BASE_DEPS_LINUX_QT6}|linux/amd64|cmake|6|/opt/Qt/6.8.0/gcc_64||linux-amd64||||"
    [linux-qt5]="linux-qt5|linux-qt5|Dockerfile.core|${CORE_BASE_DEPS_LINUX_QT5}|linux/amd64|cmake|5|||linux-amd64||||"
    [win-qt6]="win-qt6|win-qt6|Dockerfile.core|${CORE_BASE_DEPS_WIN64_QT6}||cmake|6|/opt/Qt/6.8.0/mingw_64|/opt/x86_64-w64-mingw32.cmake|win64|x86_64-w64-mingw32|pei-x86-64|seh-1"
    [win-qt5]="win-qt5|win-qt5|Dockerfile.core|${CORE_BASE_DEPS_WIN_QT5}||i686-w64-mingw32-cmake|5|||win32|i686-w64-mingw32|pei-i386|dw2-1"
    [arm64-qt6]="arm64-qt6|arm64-qt6|Dockerfile.core|${CORE_BASE_DEPS_ARM64_QT6}|linux/arm64|cmake|6|/opt/Qt/6.8.5/gcc_arm64||linux-arm64||||"
    [arm64-qt5]="arm64-qt5|arm64-qt5|Dockerfile.core|${CORE_BASE_DEPS_ARM64_QT5}|linux/arm64|cmake|5|||linux-arm64||||"
    [armv7-qt5]="armv7-qt5|armv7-qt5|Dockerfile.core|${CORE_BASE_DEPS_ARMV7_QT5}|linux/arm/v7|cmake|5|||linux-armv7||||"
)

source "$APP_ROOT/.ci/scripts/docker-lib.sh"

# PLATFORMS_LIST — какой env ожидает lib; клиент не заморачивается
core_docker_main "$@"