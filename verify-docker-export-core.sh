#!/usr/bin/env bash
# verify-docker-export-core.sh — end-to-end проверка экспортированного Core SDK
#
#   linux  : структура + libs + SONAME + ELF machine + find_package + build + run
#   win    : структура + .dll/.dll.a + find_package + build + objdump (+ wine)
#   arm64  : структура + libs + SONAME + ELF machine (если собраны, иначе SKIP)
#   armv7  : то же
#
# Использование:
#   ./verify-docker-export-core.sh                              # всё, что есть в SDK
#   PLATFORMS="linux win"   ./verify-docker-export-core.sh       # без arm
#   PLATFORMS="arm64 armv7" ./verify-docker-export-core.sh       # только arm (без consumer)
#   SKIP_WINE=1             ./verify-docker-export-core.sh       # без wine
set -euo pipefail

# ── force C locale для readelf/awk/sed/grep — иначе в RU-локали
# readelf печатает "Машина:" и парсинг ломается
export LC_ALL=C LANG=C

CORE_ROOT="$(cd "$(dirname "$0")" && pwd)"
DEPLOY_ROOT="${DEPLOY_ROOT:-$HOME/deploy/core}"
CORE_VERSION="${CORE_VERSION:-$(tr -d '[:space:]' < "$CORE_ROOT/VERSION")}"
PLATFORMS="${PLATFORMS:-linux win arm64 armv7}"
WIN_IMAGE="${WIN_IMAGE:-core-builder-cmake-win:latest}"

SDK_ROOT="$DEPLOY_ROOT/$CORE_VERSION"
WORK_DIR="$(mktemp -d -t verify-deploy.XXXXXX)"
trap 'rm -rf "$WORK_DIR"' EXIT

# ── цвета / счётчики ──────────────────────────────────────
if [ -t 1 ]; then
    RED=$'\033[0;31m'; GREEN=$'\033[0;32m'; YELLOW=$'\033[1;33m'
    BLUE=$'\033[0;34m'; NC=$'\033[0m'
else
    RED=''; GREEN=''; YELLOW=''; BLUE=''; NC=''
fi
PASS=0; FAIL=0; SKIP=0

_pass() { PASS=$((PASS+1)); printf '  %s✓%s %s\n' "$GREEN" "$NC" "$1"; }
_fail() { FAIL=$((FAIL+1)); printf '  %s✗%s %s\n' "$RED"   "$NC" "$1"; }
_skip() { SKIP=$((SKIP+1)); printf '  %s○%s %s\n' "$YELLOW" "$NC" "$1"; }
_info() { printf '  %s·%s %s\n' "$BLUE" "$NC" "$1"; }
_section() { printf '\n%s▶ %s%s\n' "$BLUE" "$1" "$NC"; }

check_dir()  { [ -d "$1" ] && _pass "dir  ${1#$SDK_ROOT/}" || _fail "dir  ${1#$SDK_ROOT/}"; }
check_file() { [ -f "$1" ] && _pass "file ${1#$SDK_ROOT/}" || _fail "file ${1#$SDK_ROOT/}"; }
check_glob() { compgen -G "$1" >/dev/null && _pass "${2:-$1}" || _fail "${2:-$1}"; }
check_nonempty_dir() { [ -d "$1" ] && [ -n "$(ls -A "$1" 2>/dev/null)" ] \
                         && _pass "headers ${1#$SDK_ROOT/}" \
                         || _fail "headers ${1#$SDK_ROOT/} (пусто или нет)"; }

# пустой ли каталог (нет или внутри нет ни одного файла рекурсивно)
is_empty_platform() {
    local p="$1"
    [ ! -d "$SDK_ROOT/$p" ] && return 0
    [ -z "$(find "$SDK_ROOT/$p" -type f -o -type l 2>/dev/null | head -1)" ] && return 0
    return 1
}

elf_expected_machine() {
    case "$1" in
        linux)  echo "Advanced Micro Devices X86-64" ;;
        arm64)  echo "AArch64" ;;
        armv7)  echo "ARM" ;;
        *)      echo "" ;;
    esac
}

check_elf_machine() {
    local file="$1" expected="$2" label="$3"
    # LC_ALL=C уже установлен глобально — readelf печатает "Machine:"
    local machine
    machine=$(readelf -h "$file" 2>/dev/null \
        | awk -F: '/^[[:space:]]*Machine:/{sub(/^[ \t]+/,"",$2); print $2}')
    if [ "$machine" = "$expected" ]; then
        _pass "arch $label: $machine"
    else
        _fail "arch $label: '$machine' (ожидалось '$expected')"
    fi
}

check_elf_soname() {
    local file="$1" expected="$2" label="$3"
    local soname
    soname=$(readelf -d "$file" 2>/dev/null | awk -F'[][]' '/SONAME/{print $2}')
    if [ "$soname" = "$expected" ]; then
        _pass "SONAME $label → $soname"
    else
        _fail "SONAME $label: '$soname' (ожидалось '$expected')"
    fi
}

echo
echo "================================================="
echo "  Core SDK verification"
echo "  version   : $CORE_VERSION"
echo "  SDK root  : $SDK_ROOT"
echo "  platforms : $PLATFORMS"
echo "================================================="

[ -d "$SDK_ROOT" ] || { echo "ERROR: $SDK_ROOT не существует — сначала ./docker-export-core.sh" >&2; exit 1; }

CORE_LIBS=(core-lib core-journal-lib core-io-lib core-modbus-lib
           core-mvc-lib core-utils-lib core-plot-lib core-widgets-lib)
NON_CORE_LIBS=(muParser qcustomplot QtWebApp)
ELF_PLATFORMS=(linux arm64 armv7)

# ============================================================
# 1. Структура SDK
# ============================================================
_section "1. Структура SDK"

for plat in $PLATFORMS; do
    _info "── $plat ──"

    if is_empty_platform "$plat"; then
        if [ -d "$SDK_ROOT/$plat" ]; then
            _skip "platform $plat пуст — не собран (BUILD_ARM=1 ./docker-export-core.sh для arm)"
        else
            _skip "platform $plat отсутствует в $SDK_ROOT"
        fi
        continue
    fi

    check_dir "$SDK_ROOT/$plat/include"
    check_dir "$SDK_ROOT/$plat/lib"
    check_dir "$SDK_ROOT/$plat/lib/cmake/Core"
    for lib in "${CORE_LIBS[@]}" "${NON_CORE_LIBS[@]}"; do
        check_nonempty_dir "$SDK_ROOT/$plat/include/$lib"
    done
    check_file "$SDK_ROOT/$plat/lib/cmake/Core/CoreConfig.cmake"
    check_file "$SDK_ROOT/$plat/lib/cmake/Core/CoreConfigVersion.cmake"
    check_file "$SDK_ROOT/$plat/lib/cmake/Core/CoreTargets.cmake"
    check_file "$SDK_ROOT/$plat/lib/cmake/Core/CoreTargets-release.cmake"

    # ── ELF-платформы: linux / arm64 / armv7 ──
    if [[ " ${ELF_PLATFORMS[*]} " == *" $plat "* ]]; then
        expected_arch="$(elf_expected_machine "$plat")"
        for lib in "${CORE_LIBS[@]}" qcustomplot QtWebApp; do
            check_glob "$SDK_ROOT/$plat/lib/lib${lib}.so"   "$plat/lib/lib${lib}.so"
            check_glob "$SDK_ROOT/$plat/lib/lib${lib}.so.*" "$plat/lib/lib${lib}.so.*"
        done
        check_file "$SDK_ROOT/$plat/lib/libmuParser.a"

        if command -v readelf >/dev/null; then
            for lib in "${CORE_LIBS[@]}" qcustomplot QtWebApp; do
                real=$(find "$SDK_ROOT/$plat/lib" -maxdepth 1 -type f \
                            -name "lib${lib}.so.*" \
                            ! -name "lib${lib}.so.[0-9]" \
                            2>/dev/null | head -1)
                [ -z "$real" ] && { _fail "$plat/lib/lib${lib}.so.<version> не найден"; continue; }
                base=$(basename "$real")
                major=$(echo "$base" | sed -E "s/^lib${lib//-/\\-}\\.so\\.([0-9]+)\\..*/\\1/")
                check_elf_soname  "$real" "lib${lib}.so.${major}" "$plat/lib/$base"
                [ -n "$expected_arch" ] && check_elf_machine "$real" "$expected_arch" "$plat/lib/$base"
            done
        else
            _skip "readelf не установлен — пропускаю SONAME/arch ($plat)"
        fi

        if [ "$plat" != "linux" ]; then
            _skip "$plat: consumer сборка/запуск не выполняются (только структура)"
        fi
    fi

    # ── Windows ──
    if [ "$plat" = "win" ]; then
        check_dir "$SDK_ROOT/$plat/bin"
        for lib in "${CORE_LIBS[@]}"; do
            check_file "$SDK_ROOT/$plat/bin/lib${lib}.dll"
            check_file "$SDK_ROOT/$plat/lib/lib${lib}.dll.a"
        done
        check_file "$SDK_ROOT/$plat/bin/libqcustomplot.dll"
        check_file "$SDK_ROOT/$plat/lib/libqcustomplot.dll.a"
        check_file "$SDK_ROOT/$plat/bin/libQtWebApp.dll"
        check_file "$SDK_ROOT/$plat/lib/libQtWebApp.dll.a"
        check_file "$SDK_ROOT/$plat/lib/libmuParser.a"
    fi
done

# ============================================================
# 2. Linux consumer: find_package + build + run
# ============================================================
if [[ " $PLATFORMS " == *" linux "* ]] && ! is_empty_platform linux; then
    _section "2. Linux consumer (find_package + build + run)"

    L="$WORK_DIR/linux"
    mkdir -p "$L"
    cat > "$L/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.18)
project(t_linux LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(Core CONFIG REQUIRED)
add_executable(t_linux main.cpp)
target_link_libraries(t_linux PRIVATE Core::core-lib)
EOF
    cat > "$L/main.cpp" <<'EOF'
#include <QCoreApplication>
#include <core-lib/Logger.h>
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    qCDebug(logDebug) << "LINUX CONSUMER OK";
    return 0;
}
EOF

    if cmake -S "$L" -B "$L/build" -DCMAKE_PREFIX_PATH="$SDK_ROOT/linux" \
            >"$L/cmake.log" 2>&1; then
        _pass "cmake configure"
    else
        _fail "cmake configure — лог:"
        sed 's/^/      /' "$L/cmake.log" | tail -30
    fi

    if [ -f "$L/build/Makefile" ] || [ -f "$L/build/build.ninja" ]; then
        if cmake --build "$L/build" -j"$(nproc)" >"$L/build.log" 2>&1; then
            _pass "cmake build"
        else
            _fail "cmake build — лог:"
            sed 's/^/      /' "$L/build.log" | tail -30
        fi
    fi

    if [ -x "$L/build/t_linux" ]; then
        out=$(LD_LIBRARY_PATH="$SDK_ROOT/linux/lib" "$L/build/t_linux" 2>&1 || true)
        if echo "$out" | grep -q "LINUX CONSUMER OK"; then
            _pass "run: ${out##*: }"
        else
            _fail "run: $out"
        fi
    fi
fi

# ============================================================
# 3. Win consumer: find_package + build + objdump (+ wine)
# ============================================================
if [[ " $PLATFORMS " == *" win "* ]] && ! is_empty_platform win; then
    _section "3. Win consumer (find_package + build + linkcheck${SKIP_WINE:+; wine skip})"

    if ! docker image inspect "$WIN_IMAGE" >/dev/null 2>&1; then
        _skip "образ $WIN_IMAGE не найден — пропускаю win-тест"
    else
        W="$WORK_DIR/win"
        mkdir -p "$W"

        cat > "$W/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.18)
project(t_win LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(Core CONFIG REQUIRED)
add_executable(t_win main.cpp)
target_link_libraries(t_win PRIVATE Core::core-lib)
EOF
        cat > "$W/main.cpp" <<'EOF'
#include <QCoreApplication>
#include <core-lib/Logger.h>
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    qCDebug(logDebug) << "WIN CONSUMER OK";
    return 0;
}
EOF
        cat > "$W/i686-w64-mingw32.cmake" <<'EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_C_COMPILER   i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  i686-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32 /sdk)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
EOF

        if docker run --rm \
                -v "$W":/work \
                -v "$SDK_ROOT/win":/sdk:ro \
                -w /work \
                "$WIN_IMAGE" \
                bash -c 'set -e; cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/work/i686-w64-mingw32.cmake -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"' \
                >"$W/build.log" 2>&1; then
            _pass "cmake configure + build (в контейнере)"
        else
            _fail "win build — лог:"
            sed 's/^/      /' "$W/build.log" | tail -30
        fi

        docker run --rm -v "$W":/work alpine \
            chown -R "$(id -u):$(id -g)" /work/build >/dev/null 2>&1 || true

        if [ -f "$W/build/t_win.exe" ]; then
            _pass "t_win.exe собран"
            check_win=$(docker run --rm -v "$W":/work "$WIN_IMAGE" \
                i686-w64-mingw32-objdump -p /work/build/t_win.exe 2>/dev/null || true)
            if echo "$check_win" | grep -q "DLL Name: libcore-lib.dll"; then
                _pass "t_win.exe линкован с libcore-lib.dll"
            else
                _fail "t_win.exe не линкован с libcore-lib.dll"
            fi

            if [ -z "${SKIP_WINE:-}" ] && command -v wine >/dev/null 2>&1 \
                    && dpkg -l wine32 >/dev/null 2>&1; then
                mkdir -p "$W/dlls"
                docker run --rm -v "$W/dlls":/t "$WIN_IMAGE" bash -c '
                    set +e
                    # Все mingw DLL (Qt5, libwinpthread, libssp, libzstd, libiconv, …)
                    for d in /usr/i686-w64-mingw32/bin \
                             /usr/i686-w64-mingw32/lib \
                             /usr/lib/gcc/i686-w64-mingw32; do
                        [ -d "$d" ] && find "$d" -maxdepth 2 -name "*.dll" \
                            -exec cp -a {} /t/ \; 2>/dev/null
                    done
                    exit 0' >/dev/null 2>&1 || true

                WINE_PREFIX="$WORK_DIR/wine32"
                WINEARCH=win32 WINEPREFIX="$WINE_PREFIX" wineboot -i >/dev/null 2>&1 || true

                out=$(WINEARCH=win32 WINEPREFIX="$WINE_PREFIX" \
                      WINEPATH="$SDK_ROOT/win/bin;$W/dlls" \
                      wine "$W/build/t_win.exe" 2>&1 || true)
                if echo "$out" | grep -q "WIN CONSUMER OK"; then
                    _pass "wine run: WIN CONSUMER OK"
                else
                    _fail "wine run — вывод:"
                    echo "$out" | sed 's/^/      /' | tail -15
                fi
            else
                _skip "wine не запущен (нет wine / wine32 / SKIP_WINE=1)"
            fi
        fi
    fi
fi

# ============================================================
# Итог
# ============================================================
echo
echo "================================================="
printf "  PASS: %s%d%s   FAIL: %s%d%s   SKIP: %s%d%s\n" \
    "$GREEN" "$PASS" "$NC" "$RED" "$FAIL" "$NC" "$YELLOW" "$SKIP" "$NC"
echo "================================================="

[ "$FAIL" -gt 0 ] && exit 1
exit 0

#chmod +x verify-docker-export-core.sh