#!/usr/bin/env bash
# Dev-экспорт артефактов Core из Docker в $HOME/deploy/core/${CORE_VERSION}/.
# НЕ для CI. Prod-путь — CI → Core-SDK releases.
#
# Требует: собранных базовых образов (bash docker-build-base-deps.sh)
# и (опционально) предварительно собранных core-образов
# (bash docker-build-core-cmake.sh). Если образов нет — соберёт через compose.
set -euo pipefail

cd "$(dirname "$0")"

if [ ! -f VERSION ]; then
    echo "ERROR: VERSION file not found in $(pwd)" >&2
    exit 1
fi

CORE_VERSION="$(tr -d '[:space:]' < VERSION)"
DEPLOY_ROOT="${DEPLOY_ROOT:-$HOME/deploy/core}"
OUT_ROOT="${DEPLOY_ROOT}/${CORE_VERSION}"

export CORE_VERSION
export OUT_ROOT

if [ "${BUILD_ARM:-0}" = "1" ]; then
    SERVICES=(core-linux core-arm64 core-armv7 core-win)
    PLATFORMS=(linux arm64 armv7 win)
else
    SERVICES=(core-linux core-win)
    PLATFORMS=(linux win)
fi

echo "========================================"
echo "  Core version : ${CORE_VERSION}"
echo "  Output       : ${OUT_ROOT}"
echo "  Platforms    : ${PLATFORMS[*]}"
echo "========================================"

for p in "${PLATFORMS[@]}"; do
    mkdir -p "${OUT_ROOT}/${p}"
done

echo
echo "=== [1/2] Building core images (${SERVICES[*]}) ==="
docker compose -f docker-compose.export.yml build "${SERVICES[@]}"

echo
echo "=== [2/2] Exporting artifacts ==="
for svc in "${SERVICES[@]}"; do
    echo
    echo "--- ${svc} ---"
    docker compose -f docker-compose.export.yml run --rm "${svc}"
done

echo
echo "=== Updating latest symlink ==="
ln -sfn "${OUT_ROOT}" "${DEPLOY_ROOT}/latest"

echo
echo "=== Done ==="
echo "Deployed to: ${OUT_ROOT}"
find "${OUT_ROOT}" -maxdepth 2 -type d | sort