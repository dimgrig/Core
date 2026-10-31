#!/usr/bin/env bash
# ============================================================================
# docker-export-core.sh — dev-экспорт артефактов Core из Docker
# ============================================================================
# Кладёт установленный SDK в $DEPLOY_ROOT/${CORE_VERSION}/<platform>/:
#   linux/       — Qt5 linux amd64
#   linux-qt6/   — Qt6 linux amd64
#   win/         — Qt5 windows x32
#   arm64/       — Qt5 arm64
#   arm64-qt6/   — Qt6 arm64
#   armv7/       — Qt5 armv7
#
# НЕ для CI. Prod-путь — CI → Core-SDK releases.
#
# Требует: собранных base-deps образов (bash docker-build-base-deps.sh).
#
# Переменные окружения:
#   BUILD_QT6=1        — включить Qt6-платформы (linux-qt6, arm64-qt6). Default: 1
#   BUILD_ARM=1        — включить arm64/armv7 (медленно; QEMU для armv7). Default: 0
#   DEPLOY_ROOT=…      — куда класть результаты. Default: $HOME/deploy/core
#
# Примеры:
#   ./docker-export-core.sh
#       → linux, linux-qt6, win
#
#   BUILD_ARM=1 ./docker-export-core.sh
#       → linux, linux-qt6, win, arm64, arm64-qt6, armv7
#
#   BUILD_QT6=0 ./docker-export-core.sh
#       → только Qt5: linux, win
#
#   BUILD_ARM=1 BUILD_QT6=0 ./docker-export-core.sh
#       → только Qt5: linux, win, arm64, armv7
#
#   DEPLOY_ROOT=/data/core ./docker-export-core.sh
#       → /data/core/${CORE_VERSION}/{linux,linux-qt6,win,...}
# ============================================================================

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

BUILD_ARM="${BUILD_ARM:-0}"
BUILD_QT6="${BUILD_QT6:-1}"

# ── Формируем списки сервисов и выходных каталогов ──────────
SERVICES=()
PLATFORMS=()

add() {
    SERVICES+=("$1")
    PLATFORMS+=("$2")
}

# linux (Qt5 всегда, Qt6 — по флагу)
add core-linux linux
if [ "$BUILD_QT6" = "1" ]; then
    add core-linux-qt6 linux-qt6
fi

# win — Qt5-only (Qt6-win пока нет base-образа)
add core-win win
if [ "$BUILD_QT6" = "1" ]; then
    echo "⏭️  core-win-qt6: пропущен (нет base-deps-win-qt6)"
fi

# arm
if [ "$BUILD_ARM" = "1" ]; then
    add core-arm64 arm64
    if [ "$BUILD_QT6" = "1" ]; then
        add core-arm64-qt6 arm64-qt6
    fi
    add core-armv7 armv7
    if [ "$BUILD_QT6" = "1" ]; then
        echo "⏭️  core-armv7-qt6: пропущен (base-deps-armv7 = Qt5-only)"
    fi
else
    if [ "$BUILD_QT6" = "1" ]; then
        echo "⏭️  arm64-qt6/armv7: пропущены (BUILD_ARM=0)"
    fi
fi

# ── Проверка, что нужные base-образы собраны ───────────────
MISSING=()
for svc in "${SERVICES[@]}"; do
    case "$svc" in
        core-linux)         img="base-deps-linux:latest"       ;;
        core-linux-qt6)     img="base-deps-linux-qt6:latest"   ;;
        core-win)           img="base-deps-win:latest"         ;;
        core-arm64)         img="base-deps-arm64:latest"       ;;
        core-arm64-qt6)     img="base-deps-arm64-qt6:latest"   ;;
        core-armv7)         img="base-deps-armv7:latest"       ;;
        *)                  img=""                            ;;
    esac
    if [ -n "$img" ] && ! docker image inspect "$img" >/dev/null 2>&1; then
        MISSING+=("$img")
    fi
done

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "ERROR: отсутствуют base-образы:" >&2
    printf '  - %s\n' "${MISSING[@]}" >&2
    echo "Соберите: BUILD_ARM=$BUILD_ARM bash docker-build-base-deps.sh" >&2
    exit 1
fi

# ── Шапка ─────────────────────────────────────────────────
echo "========================================"
echo "  Core version : ${CORE_VERSION}"
echo "  Output       : ${OUT_ROOT}"
echo "  BUILD_QT6    : ${BUILD_QT6}"
echo "  BUILD_ARM    : ${BUILD_ARM}"
echo "  Platforms    : ${PLATFORMS[*]}"
echo "  Services     : ${SERVICES[*]}"
echo "========================================"

for p in "${PLATFORMS[@]}"; do
    mkdir -p "${OUT_ROOT}/${p}"
done

# ── [1/2] Сборка ──────────────────────────────────────────
echo
echo "=== [1/2] Building core images ==="
docker compose -f docker-compose.export.yml build "${SERVICES[@]}"

# ── [2/2] Экспорт ─────────────────────────────────────────
echo
echo "=== [2/2] Exporting artifacts ==="
for svc in "${SERVICES[@]}"; do
    echo
    echo "--- ${svc} ---"
    docker compose -f docker-compose.export.yml run --rm "${svc}"
done

# ── latest → ${CORE_VERSION} ──────────────────────────────
echo
echo "=== Updating latest symlink ==="
ln -sfn "${OUT_ROOT}" "${DEPLOY_ROOT}/latest"

# ── Итог ──────────────────────────────────────────────────
echo
echo "=== Done ==="
echo "Deployed to: ${OUT_ROOT}"
find "${OUT_ROOT}" -maxdepth 2 -type d | sort