#!/usr/bin/env bash
# ============================================================================
# Core/docker.sh — единый скрипт сборки/экспорта Core SDK.
#
# Заменяет: docker-export-core.sh, docker-ci-export-core.sh
#
# Команды:
#   base     — собрать base-deps образы (делегирует в docker-build-base-deps.sh)
#   export   — экспортировать Core SDK (dev или ci режим)
#   test     — запустить ctest в контейнере (--target test; linux-qt5/linux-qt6)
#   verify   — проверить экспортированный SDK (делегирует в verify-docker-export-core.sh)
#   mode     — показать детектированный режим (dev|ci)
#   all      — base + export + verify
#   help     — справка
#
# Режимы (auto-detect, если не задан EXPORT_MODE):
#   dev  → OUT_DIR=$HOME/deploy/core/<ver>/<platform>/  + latest symlink
#   ci   → OUT_DIR=$PWD/dist/<platform>/                + package .tar.gz/.zip
#
# Env:
#   PLATFORMS                     — пробел-разделённый список (default: linux-qt6)
#   EXPORT_MODE                   — dev | ci (auto)
#   OUT_DIR                       — override выходного каталога
#   DEPLOY_ROOT                   — override deploy root (только dev)
#   CI                            — "true" → ci в auto-detect
#   CLEAN                         — 1 → rm -rf OUT_DIR перед сборкой
#   CACHE_FROM / CACHE_TO         — buildx cache (напр. type=gha,scope=core-…)
#   QT6_INSTALL_PREFIX            — для test (--target test)
#   CORE_BASE_DEPS_LINUX_QT6      — default: base-deps-linux-qt6:latest
#   CORE_BASE_DEPS_LINUX_QT5      — default: base-deps-linux:latest
#   CORE_BASE_DEPS_WIN64_QT6      — default: base-deps-win64-qt6:latest
#   CORE_BASE_DEPS_WIN_QT5        — default: base-deps-win:latest
#   CORE_BASE_DEPS_ARM64_QT6      — default: base-deps-arm64-qt6:latest
#   CORE_BASE_DEPS_ARM64_QT5      — default: base-deps-arm64:latest
#   CORE_BASE_DEPS_ARMV7_QT5      — default: base-deps-armv7:latest
#
# PLATFORMS="linux-qt6 linux-qt5 win-qt5 win-qt6 arm64-qt6 arm64-qt5 armv7-qt5" ./docker.sh export
# ============================================================================
set -euo pipefail

CORE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() { sed -n '3,36p' "$0"; exit "${1:-0}"; }
die()   { echo "ERROR: $*" >&2; exit 1; }

# ── CoreCI submodule guard ────────────────────────────────────────
[ -d "$CORE_ROOT/.ci" ] || {
    die "CoreCI submodule not initialized. Run: git submodule update --init --recursive"
}

# ---------------------------------------------------------------------------
# detect_export_mode — выставляет EXPORT_MODE (dev|ci). Побочных эффектов нет.
# ---------------------------------------------------------------------------
source "$CORE_ROOT/.ci/scripts/detect-export-mode.sh"

# ---------------------------------------------------------------------------
# _base_deps_env — выставляет CORE_BASE_DEPS_* (default: latest).
# ---------------------------------------------------------------------------
_base_deps_env() {
    CORE_BASE_DEPS_LINUX_QT6="${CORE_BASE_DEPS_LINUX_QT6:-base-deps-linux-qt6:latest}"
    CORE_BASE_DEPS_LINUX_QT5="${CORE_BASE_DEPS_LINUX_QT5:-base-deps-linux:latest}"
    CORE_BASE_DEPS_WIN64_QT6="${CORE_BASE_DEPS_WIN64_QT6:-base-deps-win64-qt6:latest}"
    CORE_BASE_DEPS_WIN_QT5="${CORE_BASE_DEPS_WIN_QT5:-base-deps-win:latest}"
    CORE_BASE_DEPS_ARM64_QT6="${CORE_BASE_DEPS_ARM64_QT6:-base-deps-arm64-qt6:latest}"
    CORE_BASE_DEPS_ARM64_QT5="${CORE_BASE_DEPS_ARM64_QT5:-base-deps-arm64:latest}"
    CORE_BASE_DEPS_ARMV7_QT5="${CORE_BASE_DEPS_ARMV7_QT5:-base-deps-armv7:latest}"
}

_platform_conf() {
    case "$1" in
        linux-qt6) echo "linux-qt6|linux-qt6|${CORE_BASE_DEPS_LINUX_QT6}|linux/amd64|cmake|6|/opt/Qt/6.8.0/gcc_64|" ;;
        linux-qt5) echo "linux-qt5|linux-qt5|${CORE_BASE_DEPS_LINUX_QT5}|linux/amd64|cmake|5||" ;;
        win-qt6)   echo "win-qt6|win-qt6|${CORE_BASE_DEPS_WIN64_QT6}|-|cmake|6|/opt/Qt/6.8.0/mingw_64|/opt/x86_64-w64-mingw32.cmake" ;;
        win-qt5)   echo "win-qt5|win-qt5|${CORE_BASE_DEPS_WIN_QT5}|-|i686-w64-mingw32-cmake|5||" ;;
        arm64-qt6) echo "arm64-qt6|arm64-qt6|${CORE_BASE_DEPS_ARM64_QT6}|linux/arm64|cmake|6|/opt/Qt/6.8.5/gcc_arm64|" ;;
        arm64-qt5) echo "arm64-qt5|arm64-qt5|${CORE_BASE_DEPS_ARM64_QT5}|linux/arm64|cmake|5||" ;;
        armv7-qt5) echo "armv7-qt5|armv7-qt5|${CORE_BASE_DEPS_ARMV7_QT5}|linux/arm/v7|cmake|5||" ;;
        *) return 1 ;;
    esac
}

# Все guard-конструкции через `if …; then …; fi` — под `set -e` не «провалятся».
_cache_args() {
    CACHE_ARGS=()
    if [ -n "${CACHE_FROM:-}" ]; then CACHE_ARGS+=( --cache-from "$CACHE_FROM" ); fi
    if [ -n "${CACHE_TO:-}"   ]; then CACHE_ARGS+=( --cache-to   "$CACHE_TO"   ); fi
}

# ---------------------------------------------------------------------------
# _cmd_export
# ---------------------------------------------------------------------------
_cmd_export() {
    PLATFORMS="${PLATFORMS:-linux-qt6}"
    CLEAN="${CLEAN:-0}"

    local APP_VERSION
    APP_VERSION="$(tr -d '[:space:]' < VERSION)"

    detect_export_mode || exit 1

    case "$EXPORT_MODE" in
        dev) DEPLOY_ROOT="${DEPLOY_ROOT:-$HOME/deploy/core}"; OUT_DIR="${OUT_DIR:-$DEPLOY_ROOT/$APP_VERSION}" ;;
        ci)  DEPLOY_ROOT="${DEPLOY_ROOT:-}";                  OUT_DIR="${OUT_DIR:-$PWD/dist}" ;;
    esac
    echo "=== mode: ${EXPORT_MODE} (OUT_DIR=${OUT_DIR}, DEPLOY_ROOT=${DEPLOY_ROOT:-<none>}) ==="

    _base_deps_env
    if [ "$CLEAN" = "1" ]; then
        echo "=== CLEAN=1 → rm -rf $OUT_DIR ==="
        rm -rf "$OUT_DIR"
    fi
    mkdir -p "$OUT_DIR"

    echo "=== core ${APP_VERSION} → ${OUT_DIR} ==="
    echo "=== PLATFORMS: ${PLATFORMS} ==="
    read -ra _plats <<< "${PLATFORMS}"

    # pre-flight
    local MISSING=() plat conf base_img
    for plat in "${_plats[@]}"; do
        conf="$(_platform_conf "$plat")" || die "неизвестная платформа '$plat' (linux-qt6 linux-qt5 win-qt6 win-qt5 arm64-qt6 arm64-qt5 armv7-qt5)"
        conf="$(echo "$conf" | sed -E 's/ *\| */|/g; s/^\| *//; s/ *\|$//')"
        base_img="$(echo "$conf" | cut -d'|' -f3)"
        [ -z "$base_img" ] && continue

        local ok=0
        if docker image inspect "$base_img" >/dev/null 2>&1; then
            ok=1
        elif docker manifest inspect "$base_img" >/dev/null 2>&1; then
            ok=1   # exists in registry; buildx will pull it
        fi
        if [ "$ok" != "1" ]; then
            MISSING+=("$base_img  (platform $plat)")
        fi
    done
    if [ ${#MISSING[@]} -gt 0 ]; then
        echo "ERROR: base-deps образы недоступны ни локально, ни в registry:" >&2
        printf '  - %s\n' "${MISSING[@]}" >&2
        echo >&2
        echo "В dev:   соберите локально — ./docker.sh base" >&2
        echo "В CI:    проверьте, что образ опубликован (workflow: 'Build base-deps images')," >&2
        echo "         и что имя/тег в CORE_BASE_DEPS_* совпадает с workflows/base-deps.yml:" >&2
        echo "           linux-qt6:   core-base-deps-linux:qt6-bookworm" >&2
        echo "           win-qt6:     core-base-deps-win64:qt6-mingw" >&2
        echo "           arm64-qt6:   core-base-deps-arm64:qt6-bookworm-aqt" >&2
        echo "           linux-qt5:   core-base-deps-linux:qt5-bookworm" >&2
        echo "           win-qt5:     core-base-deps-win:qt5-mingw" >&2
        echo "           arm64-qt5:   core-base-deps-arm64:qt5-bookworm" >&2
        echo "           armv7-qt5:   core-base-deps-armv7:qt5-bullseye" >&2
        exit 1
    fi

    local pkg_suffix out_subdir buildx_plat cmake_cmd qt_major qt6_prefix toolchain fmt pkg
    for plat in "${_plats[@]}"; do
        conf="$(_platform_conf "$plat")"
        conf="$(echo "$conf" | sed -E 's/ *\| */|/g; s/^\| *//; s/ *\|$//')"
        IFS='|' read -r pkg_suffix out_subdir base_img buildx_plat \
                        cmake_cmd qt_major qt6_prefix toolchain <<< "$conf"

        echo
        echo "--- ${plat} (→ ${out_subdir}/) ---"

        local bargs=( --build-arg "BASE_DEPS_IMAGE=${base_img}"
                      --build-arg "CMAKE_CMD=${cmake_cmd}"
                      --build-arg "QT_VERSION_MAJOR=${qt_major}" )
        if [ -n "$qt6_prefix" ]; then bargs+=( --build-arg "QT6_INSTALL_PREFIX=${qt6_prefix}" ); fi
        if [ -n "$toolchain"  ]; then bargs+=( --build-arg "CMAKE_TOOLCHAIN_FILE=${toolchain}" ); fi
        bargs+=( --target export --output "type=local,dest=${OUT_DIR}/${out_subdir}" )
        if [ "$buildx_plat" != "-" ]; then bargs+=( --platform "$buildx_plat" ); fi

        _cache_args
        docker buildx build "${bargs[@]}" "${CACHE_ARGS[@]}" -f Dockerfile.core .

        case "$pkg_suffix" in win-*) fmt="zip" ;; *) fmt="tar.gz" ;; esac
        if [ "$EXPORT_MODE" = "ci" ]; then
            case "$pkg_suffix" in
                linux-qt6) pkg="Core-Releases-${APP_VERSION}-linux-amd64-qt6.${fmt}" ;;
                linux-qt5) pkg="Core-Releases-${APP_VERSION}-linux-amd64-qt5.${fmt}" ;;
                win-qt6)   pkg="Core-Releases-${APP_VERSION}-win64-qt6.${fmt}" ;;
                win-qt5)   pkg="Core-Releases-${APP_VERSION}-win32-qt5.${fmt}" ;;
                arm64-qt6) pkg="Core-Releases-${APP_VERSION}-linux-arm64-qt6.${fmt}" ;;
                arm64-qt5) pkg="Core-Releases-${APP_VERSION}-linux-arm64-qt5.${fmt}" ;;
                armv7-qt5) pkg="Core-Releases-${APP_VERSION}-linux-armv7-qt5.${fmt}" ;;
            esac
        else
            pkg="core-${APP_VERSION}-${pkg_suffix}.${fmt}"
        fi

        if [ "$fmt" = "zip" ]; then
            ( cd "$OUT_DIR/$out_subdir" && zip -qr "../$pkg" . )
        else
            tar -czf "$OUT_DIR/$pkg" -C "$OUT_DIR/$out_subdir" .
        fi
        echo "  → $OUT_DIR/$pkg ($(du -h "$OUT_DIR/$pkg" | cut -f1))"
    done

    if [ -n "$DEPLOY_ROOT" ]; then
        mkdir -p "$DEPLOY_ROOT"
        ln -sfn "$OUT_DIR" "${DEPLOY_ROOT}/latest"
        echo; echo "=== latest -> ${OUT_DIR} ==="
    fi

    echo; echo "=== Done ==="
    find "$OUT_DIR" -maxdepth 1 -type f \
        \( -name "Core-Releases-*.tar.gz" -o -name "Core-Releases-*.zip" \
           -o -name "core-*.tar.gz"          -o -name "core-*.zip" \) \
        -exec ls -lh {} \; 2>/dev/null || true
}

# ---------------------------------------------------------------------------
# _cmd_test — ctest в контейнере (--target test). Только linux-qt5/linux-qt6.
# ---------------------------------------------------------------------------
_cmd_test() {
    PLATFORMS="${PLATFORMS:-linux-qt6}"
    read -ra _plats <<< "${PLATFORMS}"
    _base_deps_env

    local tested=0 plat base_img qt_major
    for plat in "${_plats[@]}"; do
        case "$plat" in
            linux-qt6) base_img="$CORE_BASE_DEPS_LINUX_QT6"; qt_major=6 ;;
            linux-qt5) base_img="$CORE_BASE_DEPS_LINUX_QT5"; qt_major=5 ;;
            *) echo "NOTE: test supports only linux-qt5/linux-qt6, skipping '$plat'"; continue ;;
        esac

        echo
        echo "--- test ${plat} (base: ${base_img}) ---"
        local targs=( --target test
                      --build-arg "BASE_DEPS_IMAGE=${base_img}"
                      --build-arg "CMAKE_CMD=cmake"
                      --build-arg "QT_VERSION_MAJOR=${qt_major}" )
        if [ -n "${QT6_INSTALL_PREFIX:-}" ]; then
            targs+=( --build-arg "QT6_INSTALL_PREFIX=${QT6_INSTALL_PREFIX}" )
        fi

        _cache_args
        docker buildx build "${targs[@]}" "${CACHE_ARGS[@]}" -f Dockerfile.core .
        tested=$((tested + 1))
    done
    [ "$tested" -gt 0 ] || die "no linux-qt5/linux-qt6 in PLATFORMS='$PLATFORMS'"
    echo
    echo "=== test OK (${tested} platform(s)) ==="
}

# ---------------------------------------------------------------------------
# _main
# ---------------------------------------------------------------------------
_main() {
    cd "$CORE_ROOT"
    [ -f VERSION ] || die "VERSION file not found in $CORE_ROOT"

    local cmd="${1:-}"; shift || true
    case "$cmd" in
        base)   bash "$CORE_ROOT/.ci/scripts/build-base-deps.sh" "$@" ;;
        verify) bash ./verify-docker-export-core.sh "$@" ;;
        export) _cmd_export "$@" ;;
        test)   _cmd_test   "$@" ;;
        mode)   detect_export_mode || exit 1; echo "$EXPORT_MODE" ;;
        all)    bash ./docker.sh base; bash ./docker.sh export "$@"; bash ./docker.sh verify ;;
        help|-h|--help) usage 0 ;;
        "")     usage 1 ;;
        *)      die "неизвестная команда '$cmd' (base|export|test|verify|mode|all|help)" ;;
    esac
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    _main "$@"
fi