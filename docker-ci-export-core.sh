#!/usr/bin/env bash
# ============================================================================
# docker-ci-export-core.sh — CI-экспорт SDK через docker buildx --output type=local.
# НЕ для локальной разработки — используй ./docker-export-core.sh.
#
# PLATFORMS (пробел-разделённый список), по умолчанию "linux-qt6":
#   linux-qt6   linux-qt5   win-qt6   win-qt5   arm64-qt6   arm64-qt5   armv7-qt5
#
# Env:
#   OUT_DIR   — куда писать (default: $PWD/dist)
#   PLATFORMS — список платформ (default: "linux-qt6")
# ============================================================================

set -euo pipefail
cd "$(dirname "$0")"

CORE_VERSION="$(tr -d '[:space:]' < VERSION)"
OUT_DIR="${OUT_DIR:-$PWD/dist}"
PLATFORMS="${PLATFORMS:-linux-qt6}"

mkdir -p "$OUT_DIR"

# ── platform → pkg_suffix | out_dir | base_deps_image | buildx_platform |
#                cmake_cmd | core_install_base | qt_major |
#                qt6_install_prefix | cmake_toolchain_file ──
# out_dir — обратная совместимость с docker-export-core.sh:
#   Qt5 → linux/win/arm64/armv7  |  Qt6 → linux-qt6/win-qt6/arm64-qt6
platform_conf() {
    case "$1" in
        linux-qt6) echo "linux-qt6 |linux-qt6|base-deps-linux-qt6:latest    |linux/amd64  |cmake                 |/opt/core                  |6|/opt/Qt/6.8.0/gcc_64  |" ;;
        linux-qt5) echo "linux-qt5 |linux    |base-deps-linux:latest        |linux/amd64  |cmake                 |/opt/core                  |5|                       |" ;;
        win-qt6)   echo "win-qt6   |win-qt6  |base-deps-win64-qt6:latest    |-            |cmake                 |/app/Core/applocal-staging |6|/opt/Qt/6.8.0/mingw_64 |/opt/x86_64-w64-mingw32.cmake" ;;
        win-qt5)   echo "win-qt5   |win      |base-deps-win:latest          |-            |i686-w64-mingw32-cmake|/app/Core/applocal-staging |5|                       |" ;;
        arm64-qt6) echo "arm64-qt6 |arm64-qt6|base-deps-arm64-qt6:latest    |linux/arm64  |cmake                 |/opt/core                  |6|/opt/Qt/6.8.5/gcc_arm64|" ;;
        arm64-qt5) echo "arm64-qt5 |arm64    |base-deps-arm64:latest        |linux/arm64  |cmake                 |/opt/core                  |5|                       |" ;;
        armv7-qt5) echo "armv7-qt5 |armv7    |base-deps-armv7:latest        |linux/arm/v7 |cmake                 |/opt/core                  |5|                       |" ;;
        *)         return 1 ;;
    esac
}

echo "=== Core ${CORE_VERSION} → ${OUT_DIR} ==="
echo "=== PLATFORMS: ${PLATFORMS} ==="

read -ra _plats <<< "${PLATFORMS}"

for plat in "${_plats[@]}"; do
    if ! conf="$(platform_conf "$plat")"; then
        echo "ERROR: неизвестная платформа '$plat'" >&2
        echo "  допустимые: linux-qt6 linux-qt5 win-qt6 win-qt5 arm64-qt6 arm64-qt5 armv7-qt5" >&2
        exit 1
    fi

    # Убираем пробелы вокруг полей (форматирование |field| для читаемости)
    conf="$(echo "$conf" | sed -E 's/ *\| */|/g; s/^\| *//; s/ *\|$//')"

    IFS='|' read -r pkg_suffix out_subdir base_img buildx_plat cmake_cmd \
                    install_base qt_major qt6_prefix toolchain <<< "$conf"

    echo
    echo "--- ${plat} (→ ${out_subdir}/, pkg ${pkg_suffix}) ---"

    bargs=( --build-arg "BASE_DEPS_IMAGE=${base_img}"
            --build-arg "CMAKE_CMD=${cmake_cmd}"
            --build-arg "QT_VERSION_MAJOR=${qt_major}"
            --build-arg "CORE_INSTALL_BASE=${install_base}" )

    [ -n "$qt6_prefix" ] && bargs+=( --build-arg "QT6_INSTALL_PREFIX=${qt6_prefix}" )
    [ -n "$toolchain"  ] && bargs+=( --build-arg "CMAKE_TOOLCHAIN_FILE=${toolchain}" )

    bargs+=( --target export
             --output "type=local,dest=${OUT_DIR}/${out_subdir}" )

    [ "$buildx_plat" != "-" ] && bargs+=( --platform "$buildx_plat" )

    docker buildx build "${bargs[@]}" -f Dockerfile.core .

    case "$pkg_suffix" in
        win-*) fmt="zip"    ;;
        *)     fmt="tar.gz" ;;
    esac
    pkg="core-${CORE_VERSION}-${pkg_suffix}.${fmt}"

    if [ "$fmt" = "zip" ]; then
        ( cd "$OUT_DIR/$out_subdir" && zip -qr "../$pkg" . )
    else
        tar -czf "$OUT_DIR/$pkg" -C "$OUT_DIR/$out_subdir" .
    fi
    echo "  → $OUT_DIR/$pkg ($(du -h "$OUT_DIR/$pkg" | cut -f1))"
done

echo
echo "=== Done ==="
ls -lh "$OUT_DIR"/core-*.{tar.gz,zip} 2>/dev/null