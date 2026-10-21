#!/bin/bash
set -e  # останавливаться при любой ошибке

# Переходим в корень проекта (откуда запущен скрипт)
cd "$(dirname "$0")" || exit

echo "=== Сборка базовых образов ==="
docker build -t base-deps-linux:latest -f ../Core/Dockerfile.base-deps-linux ..
echo "✅ base-deps-linux:latest"
docker build -t base-deps-win:latest   -f ../Core/Dockerfile.base-deps-win ..
echo "✅ base-deps-win:latest"
docker buildx build --platform linux/arm64 -t base-deps-arm64:latest -f ../Core/Dockerfile.base-deps-arm64 ..
echo "✅ base-deps-arm64:latest"
docker buildx build --platform linux/arm/v7 -t base-deps-armv7:latest -f ../Core/Dockerfile.base-deps-armv7 ..
echo "✅ base-deps-armv7:latest"
docker build -t innosetup-base:latest  -f ../Core/Dockerfile.innosetup-base ..
echo "✅ innosetup-base:latest"

: '
if ! docker image inspect base-deps-linux:latest >/dev/null 2>&1; then
    docker build -t base-deps-linux:latest -f ../Core/Dockerfile.base-deps-linux ..
    echo "✅ base-deps-linux:latest (собран)"
else
    echo "⏭️  base-deps-linux:latest (уже есть, пропуск)"
fi

if ! docker image inspect base-deps-win:latest >/dev/null 2>&1; then
    docker build -t base-deps-win:latest -f ../Core/Dockerfile.base-deps-win ..
    echo "✅ base-deps-win:latest (собран)"
else
    echo "⏭️  base-deps-win:latest (уже есть, пропуск)"
fi

#if ! docker image inspect base-deps-arm64:latest >/dev/null 2>&1; then
    docker buildx build --platform linux/arm64 -t base-deps-arm64:latest -f ../Core/Dockerfile.base-deps-arm64 ..
    echo "✅ base-deps-arm64:latest (собран)"
else
    echo "⏭️  base-deps-arm64:latest (уже есть, пропуск)"
fi

if ! docker image inspect base-deps-armv7:latest >/dev/null 2>&1; then
    docker buildx build --platform linux/arm/v7 -t base-deps-armv7:latest -f ../Core/Dockerfile.base-deps-armv7 ..
    echo "✅ base-deps-armv7:latest (собран)"
else
    echo "⏭️  base-deps-armv7:latest (уже есть, пропуск)"
fi

if ! docker image inspect innosetup-base:latest >/dev/null 2>&1; then
    docker build -t innosetup-base:latest -f ../Core/Dockerfile.innosetup-base ..
    echo "✅ innosetup-base:latest (собран)"
else
    echo "⏭️  innosetup-base:latest (уже есть, пропуск)"
fi 
'