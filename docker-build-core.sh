#!/bin/bash
set -e  # останавливаться при любой ошибке

# Переходим в корень проекта Core
cd "$(dirname "$0")/../Core" || exit

bash ../Core/docker-build-base-deps.sh

echo "=== Сборка Core ==="
echo "=== Сборка core-builder-linux:latest ==="
docker build -f Dockerfile.core -t core-builder-linux ..
echo "✅ core-builder-linux:latest"


echo "=== Сборка core-builder-win:latest ==="
docker build -f Dockerfile.core -t core-builder-win \
    --build-arg BASE_DEPS_IMAGE=base-deps-win \
    --build-arg QMAKE_SPEC=win32-g++ \
    --build-arg QMAKE_BIN=i686-w64-mingw32-qmake-qt5 \
    ..
echo "✅ core-builder-win:latest"


echo "=== Сборка core-builder-arm64:latest ==="
docker buildx build --platform linux/arm64 \
    -f Dockerfile.core -t core-builder-arm64 \
    --build-arg BASE_DEPS_IMAGE=base-deps-arm64 \
    --load ..
echo "✅ core-builder-arm64:latest"


echo "=== Сборка core-builder-armv7:latest ==="
docker buildx build --platform linux/arm/v7 \
    -f Dockerfile.core -t core-builder-armv7 \
    --build-arg BASE_DEPS_IMAGE=base-deps-armv7 \
    --load ..
echo "✅ core-builder-armv7:latest"   

#docker build -f Dockerfile.core --target test -t core-tests-linux .