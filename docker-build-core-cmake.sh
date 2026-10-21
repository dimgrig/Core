#!/bin/bash
set -e  # останавливаться при любой ошибке

# Переходим в корень проекта Core
cd "$(dirname "$0")/../Core" || exit

bash ../Core/docker-build-base-deps.sh

echo "=== Сборка Core ==="
echo "=== Сборка core-builder-linux:latest ==="
docker build \
  --build-arg BASE_DEPS_IMAGE=base-deps-linux:latest \
  --build-arg CMAKE_CMD=cmake \
  -t core-builder-cmake-linux:latest \
  -f Dockerfile.cmake ..
echo "✅ core-builder-cmake-linux:latest"


echo "=== Сборка core-builder-win:latest ==="
docker build \
  --build-arg BASE_DEPS_IMAGE=base-deps-win:latest \
  --build-arg CMAKE_CMD=i686-w64-mingw32-cmake \
  -t core-builder-cmake-win:latest \
  -f Dockerfile.cmake ..  
echo "✅ core-builder-cmake-win:latest"


# Проверка: buildx обязателен для --platform
if ! docker buildx version >/dev/null 2>&1; then
    echo "ERROR: docker buildx plugin not found. Install:"
    echo "  https://docs.docker.com/build/install-buildx/"
    exit 1
fi

<< 'COMMENT'
echo "=== core-builder-arm64 ==="
docker buildx build \
  --platform linux/arm64 \
  --load \
  --build-arg BASE_DEPS_IMAGE=base-deps-arm64:latest \
  --build-arg CMAKE_CMD=cmake \
  -t core-builder-cmake-arm64:latest \
  -f Dockerfile.cmake ..
echo "✅ core-builder-cmake-arm64:latest"


echo "=== core-builder-armv7 ==="
docker buildx build \
  --platform linux/arm/v7 \
  --load \
  --build-arg BASE_DEPS_IMAGE=base-deps-armv7:latest \
  --build-arg CMAKE_CMD=cmake \
  -t core-builder-cmake-armv7:latest \
  -f Dockerfile.cmake ..
echo "✅ core-builder-cmake-armv7:latest"
COMMENT