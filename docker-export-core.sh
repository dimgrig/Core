#!/usr/bin/env bash
# ============================================================================
# docker-export-core.sh — dev-экспорт артефактов Core из Docker
# ============================================================================
# PLATFORMS (пробел-разделённый список), по умолчанию "linux-qt6":
#   linux-qt6   → ${OUT_ROOT}/linux-qt6/
#   linux-qt5   → ${OUT_ROOT}/linux/       (LEGACY)
#   win-qt6     → ${OUT_ROOT}/win-qt6/
#   win-qt5     → ${OUT_ROOT}/win/         (LEGACY)
#   arm64-qt6   → ${OUT_ROOT}/arm64-qt6/
#   arm64-qt5   → ${OUT_ROOT}/arm64/       (LEGACY)
#   armv7-qt5   → ${OUT_ROOT}/armv7/       (LEGACY)
#
# Env:
#   PLATFORMS     — список платформ (default: "linux-qt6")
#   DEPLOY_ROOT   — куда писать (default: $HOME/deploy/core)
#
# Примеры:
#   ./docker-export-core.sh
#       → linux-qt6
#
#   PLATFORMS="linux-qt6 win-qt6" ./docker-export-core.sh
#
#   PLATFORMS="linux-qt6 win-qt6" ./docker-export-core.sh
#
#   PLATFORMS="linux-qt6 win-qt6 arm64-qt6" ./docker-export-core.sh
#
#   PLATFORMS="linux-qt6 linux-qt5" ./docker-export-core.sh
#
#   PLATFORMS="linux-qt6 linux-qt5 win-qt5 win-qt6 arm64-qt6 arm64-qt5 armv7-qt5" ./docker-export-core.sh
#       → все платформы (armv7 через QEMU, медленно)
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

PLATFORMS="${PLATFORMS:-linux-qt6}"

# ── platform → (compose_service | out_dir | base_deps_image) ──
# out_dir — каталог в ${OUT_ROOT}/, куда docker-compose.export.yml монтирует
# /out. Для Qt5-платформ сохранены старые имена (linux/win/arm64/armv7) —
# обратная совместимость с docker-compose.export.yml и Saks CMakeLists.
platform_info() {
    case "$1" in
        linux-qt6) echo "core-linux-qt6  linux-qt6  base-deps-linux-qt6:latest"  ;;
        linux-qt5) echo "core-linux      linux      base-deps-linux:latest"      ;;
        win-qt6)   echo "core-win-qt6    win-qt6    base-deps-win64-qt6:latest"  ;;
        win-qt5)   echo "core-win        win        base-deps-win:latest"        ;;
        arm64-qt6) echo "core-arm64-qt6  arm64-qt6  base-deps-arm64-qt6:latest"  ;;
        arm64-qt5) echo "core-arm64      arm64      base-deps-arm64:latest"      ;;
        armv7-qt5) echo "core-armv7      armv7      base-deps-armv7:latest"      ;;
        *)         return 1 ;;
    esac
}

read -ra _plats <<< "${PLATFORMS}"

SERVICES=()
OUT_DIRS=()
BASE_IMAGES=()

for p in "${_plats[@]}"; do
    if ! info="$(platform_info "$p")"; then
        echo "ERROR: неизвестная платформа '$p'" >&2
        echo "  допустимые: linux-qt6 linux-qt5 win-qt6 win-qt5 arm64-qt6 arm64-qt5 armv7-qt5" >&2
        exit 1
    fi
    read -r svc dir img <<< "$info"
    SERVICES+=("$svc")
    OUT_DIRS+=("$dir")
    BASE_IMAGES+=("$img")
done

MISSING=()
for img in "${BASE_IMAGES[@]}"; do
    docker image inspect "$img" >/dev/null 2>&1 || MISSING+=("$img")
done
if [ ${#MISSING[@]} -gt 0 ]; then
    echo "ERROR: отсутствуют base-образы:" >&2
    printf '  - %s\n' "${MISSING[@]}" >&2
    echo "Соберите: bash docker-build-base-deps.sh" >&2
    exit 1
fi

echo "========================================"
echo "  Core version : ${CORE_VERSION}"
echo "  Output       : ${OUT_ROOT}"
echo "  Platforms    : ${_plats[*]}"
echo "  Services     : ${SERVICES[*]}"
echo "========================================"

for d in "${OUT_DIRS[@]}"; do
    mkdir -p "${OUT_ROOT}/${d}"
done

echo
echo "=== [1/2] Building core images ==="
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
echo "Deployed to: ${DEPLOY_ROOT}"
find "${OUT_ROOT}" -maxdepth 2 -type d | sort