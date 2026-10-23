#!/bin/bash
set -e  # останавливаться при любой ошибке

# Переходим в корень проекта (откуда запущен скрипт)
cd "$(dirname "$0")" || exit

echo "=== Сборка базовых образов ==="
docker build -t base-deps-linux:latest -f Dockerfile.base-deps-linux ..
echo "✅ base-deps-linux:latest"
docker build -t base-deps-win:latest   -f Dockerfile.base-deps-win ..
echo "✅ base-deps-win:latest"
docker buildx build --platform linux/arm64 -t base-deps-arm64:latest -f Dockerfile.base-deps-arm64 ..
echo "✅ base-deps-arm64:latest"
docker buildx build --platform linux/arm/v7 -t base-deps-armv7:latest -f Dockerfile.base-deps-armv7 ..
echo "✅ base-deps-armv7:latest"
#docker build -t innosetup-base:latest  -f Dockerfile.innosetup-base ..
#echo "✅ innosetup-base:latest"

# Astra — отдельно, если нужна (требует epicmorg/astralinux:1.8-main)
if docker image inspect epicmorg/astralinux:1.8-main >/dev/null 2>&1 || \
   docker pull epicmorg/astralinux:1.8-main >/dev/null 2>&1; then
    docker build -t base-deps-astra:latest -f Dockerfile.base-deps-astra .
    echo "✅ base-deps-astra:latest"
else
    echo "⏭️  base-deps-astra: пропущен (epicmorg/astralinux недоступен)"
fi

# ── Local-only образы (для сборки installer'ов, не для SDK) ──
if [ -d "../deb/innosetup-portable" ]; then
    docker build -t innosetup-base:latest -f ../Core/Dockerfile.innosetup-base ..
    echo "✅ innosetup-base:latest (local-only)"
else
    echo "⏭️  innosetup-base: пропущен (нет ../deb/innosetup-portable)"
fi

: '
if ! docker image inspect base-deps-linux:latest >/dev/null 2>&1; then
    docker build -t base-deps-linux:latest -f Dockerfile.base-deps-linux ..
    echo "✅ base-deps-linux:latest (собран)"
else
    echo "⏭️  base-deps-linux:latest (уже есть, пропуск)"
fi

if ! docker image inspect base-deps-win:latest >/dev/null 2>&1; then
    docker build -t base-deps-win:latest -f Dockerfile.base-deps-win ..
    echo "✅ base-deps-win:latest (собран)"
else
    echo "⏭️  base-deps-win:latest (уже есть, пропуск)"
fi

#if ! docker image inspect base-deps-arm64:latest >/dev/null 2>&1; then
    docker buildx build --platform linux/arm64 -t base-deps-arm64:latest -f Dockerfile.base-deps-arm64 ..
    echo "✅ base-deps-arm64:latest (собран)"
else
    echo "⏭️  base-deps-arm64:latest (уже есть, пропуск)"
fi

if ! docker image inspect base-deps-armv7:latest >/dev/null 2>&1; then
    docker buildx build --platform linux/arm/v7 -t base-deps-armv7:latest -f Dockerfile.base-deps-armv7 ..
    echo "✅ base-deps-armv7:latest (собран)"
else
    echo "⏭️  base-deps-armv7:latest (уже есть, пропуск)"
fi

if ! docker image inspect innosetup-base:latest >/dev/null 2>&1; then
    docker build -t innosetup-base:latest -f Dockerfile.innosetup-base ..
    echo "✅ innosetup-base:latest (собран)"
else
    echo "⏭️  innosetup-base:latest (уже есть, пропуск)"
fi 
'

echo
echo "=== Готово ==="