# Определение платформы/компилятора/процессора — аналог qmake-target-platform.pri
if(WIN32)
    set(CORE_PLATFORM "windows")
    if(MINGW)
        set(CORE_COMPILER "gcc")
    elseif(MSVC)
        set(CORE_COMPILER "msvc2017")
    endif()
elseif(APPLE)
    set(CORE_PLATFORM "osx")
    set(CORE_COMPILER "clang")
elseif(UNIX)
    set(CORE_PLATFORM "linux")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CORE_COMPILER "gcc")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CORE_COMPILER "clang")
    endif()
endif()

# ── Архитектура target (не host) ────────────────────────────────
# CMAKE_SYSTEM_PROCESSOR на Windows берётся из %PROCESSOR_ARCHITECTURE%
# хоста → "AMD64" даже при сборке mingw81_32 / MSVC x86. Разрядность
# берём из CMAKE_SIZEOF_VOID_P (try_compile от компилятора):
#   4 = 32-bit, 8 = 64-bit.
# Семейство (x86 / arm) — из CMAKE_SYSTEM_PROCESSOR (совпадает во всех
# вариантах: AMD64/x86_64/i686 → x86 family; aarch64/arm64/armv7 → arm).
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_core_bits 64)
else()
    set(_core_bits 32)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    if(_core_bits EQUAL 64)
        set(CORE_PROCESSOR "arm64")
    else()
        set(CORE_PROCESSOR "arm")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(CORE_PROCESSOR "arm")
else()
    # x86 family (x86_64 / AMD64 / i686 / i386 / …)
    if(_core_bits EQUAL 64)
        set(CORE_PROCESSOR "x64")
    else()
        set(CORE_PROCESSOR "x86")
    endif()
endif()

unset(_core_bits)

# build_path (debug / release) в нижнем регистре, как в qmake
string(TOLOWER "${CMAKE_BUILD_TYPE}" CORE_BUILD_TYPE)
if(NOT CORE_BUILD_TYPE)
    set(CORE_BUILD_TYPE "release")
endif()

set(CORE_DESTINATION_PATH "${CORE_PLATFORM}/${CORE_COMPILER}/${CORE_PROCESSOR}/${CORE_BUILD_TYPE}")