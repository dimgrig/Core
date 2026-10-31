#!/bin/bash
set -e
cd "$(dirname "$0")" || exit

: <<'COMMENT'
Примеры:
./docker-build-base-deps.sh
COMMENT

echo "=== Qt5 base images ==="
docker build -t base-deps-linux:latest -f Dockerfile.base-deps-linux ..
docker build -t base-deps-win:latest   -f Dockerfile.base-deps-win ..
docker buildx build --platform linux/arm64 -t base-deps-arm64:latest -f Dockerfile.base-deps-arm64 ..
docker buildx build --platform linux/arm/v7 -t base-deps-armv7:latest -f Dockerfile.base-deps-armv7 ..

echo "=== Qt6 base images ==="
docker build -t base-deps-linux-qt6:latest -f Dockerfile.base-deps-linux-qt6 ..
# win-qt6 — отложен
docker buildx build --platform linux/arm64 -t base-deps-arm64-qt6:latest -f Dockerfile.base-deps-arm64-qt6 ..

if [ "${BUILD_ASTRA:-0}" = "1" ]; then
    docker build -t base-deps-astra:latest -f Dockerfile.base-deps-astra .
fi
if [ "${BUILD_INNOSETUP:-0}" = "1" ] && [ -d "../deb/innosetup-portable" ]; then
    docker build -t innosetup-base:latest -f ../Core/Dockerfile.innosetup-base ..
fi

echo "=== Готово ==="