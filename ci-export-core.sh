#!/usr/bin/env bash
# ci-export-core.sh — CI-экспорт SDK через docker buildx --output type=local.
# НЕ для локальной разработки — используй ./docker-export-core.sh.
#
# Env:
#   OUT_DIR   — куда писать (default: ./dist)
#   PLATFORMS — список: linux/amd64 linux/arm64 linux/arm/v7 win32 (default: все)
set -euo pipefail

cd "$(dirname "$0")"

CORE_VERSION="$(tr -d '[:space:]' < VERSION)"
OUT_DIR="${OUT_DIR:-$PWD/dist}"
PLATFORMS="${PLATFORMS:-linux/amd64 linux/arm64 linux/arm/v7 win32}"

mkdir -p "$OUT_DIR"

# platform → (суффикс каталога, base-deps-image, buildx-platform, CMAKE_CMD, CORE_INSTALL_BASE)
platform_conf() {
    case "$1" in
        linux/amd64)   echo "linux-x64   base-deps-linux:latest  linux/amd64    cmake                  /opt/core" ;;
        linux/arm64)   echo "linux-arm64 base-deps-arm64:latest  linux/arm64    cmake                  /opt/core" ;;
        linux/arm/v7)  echo "linux-armv7 base-deps-armv7:latest  linux/arm/v7   cmake                  /opt/core" ;;
        win32)         echo "win32-mingw base-deps-win:latest    -              i686-w64-mingw32-cmake /app/Core/applocal-staging" ;;
        *) return 1 ;;
    esac
}

echo "=== Core ${CORE_VERSION} → ${OUT_DIR} ==="

for plat in $PLATFORMS; do
    read -r suffix base_img buildx_plat cmake_cmd install_base <<<"$(platform_conf "$plat")"

    echo
    echo "--- ${plat} (${suffix}) ---"

    bargs=( --build-arg "CORE_VERSION=${CORE_VERSION}"
            --build-arg "BASE_DEPS_IMAGE=${base_img}"
            --build-arg "CMAKE_CMD=${cmake_cmd}"
            --build-arg "CORE_INSTALL_BASE=${install_base}" )

    bargs+=( --target export
             --output "type=local,dest=${OUT_DIR}/${suffix}" )

    if [ "$buildx_plat" != "-" ]; then
        bargs+=( --platform "$buildx_plat" )
    fi

    docker buildx build "${bargs[@]}" -f Dockerfile.core ..

    # Упаковка
    case "$suffix" in
        win32-*) fmt="zip"   ;;
        *)       fmt="tar.gz";;
    esac
    pkg="core-${CORE_VERSION}-${suffix}.${fmt}"

    if [ "$fmt" = "zip" ]; then
        (cd "$OUT_DIR/$suffix" && zip -qr "../$pkg" .)
    else
        tar -czf "$OUT_DIR/$pkg" -C "$OUT_DIR/$suffix" .
    fi
    echo "  → $OUT_DIR/$pkg ($(du -h "$OUT_DIR/$pkg" | cut -f1))"
done

echo
echo "=== Done ==="
ls -lh "$OUT_DIR"/core-*.{tar.gz,zip} 2>/dev/null