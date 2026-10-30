# Аналог qmake-core-version.pri + core-paths.pri
# CORE_LINUX_LIBS_PATH / CORE_WIN_LIBS_PATH из env, иначе дефолт

if(NOT DEFINED CORE_LINUX_LIBS_PATH)
    if(DEFINED ENV{CORE_LINUX_LIBS_PATH})
        set(CORE_LINUX_LIBS_PATH "$ENV{CORE_LINUX_LIBS_PATH}")
    else()
        if(QT_VERSION_MAJOR EQUAL 6)
            set(CORE_LINUX_LIBS_PATH "/usr/lib/qt6libs")
        else()
            set(CORE_LINUX_LIBS_PATH "/usr/lib/qt")
        endif()
    endif()
endif()

if(NOT DEFINED CORE_WIN_LIBS_PATH)
    if(DEFINED ENV{CORE_WIN_LIBS_PATH})
        set(CORE_WIN_LIBS_PATH "$ENV{CORE_WIN_LIBS_PATH}")
    else()
        if(QT_VERSION_MAJOR EQUAL 6)
            set(CORE_WIN_LIBS_PATH "C:/QtCreatorProjects/libs6")
        else()
            set(CORE_WIN_LIBS_PATH "C:/QtCreatorProjects/libs")
        endif()
    endif()
endif()

if(WIN32)
    set(CORE_DEPLOY_LIBDIR "${CORE_WIN_LIBS_PATH}")
else()
    set(CORE_DEPLOY_LIBDIR "${CORE_LINUX_LIBS_PATH}")
endif()

# Astra / GCC10 (аналог core-gcc10.pri)
if(EXISTS "/etc/astra_version")
    if(DEFINED ENV{ASTRA_GCC_PATH})
        set(ASTRA_GCC_PATH "$ENV{ASTRA_GCC_PATH}")
    else()
        set(ASTRA_GCC_PATH "/home/upkd/opt/gcc-10.1.0")
    endif()
    if(DEFINED ENV{ASTRA_QT_PATH})
        set(ASTRA_QT_PATH "$ENV{ASTRA_QT_PATH}")
    else()
        set(ASTRA_QT_PATH "/opt/qt5.15.12")
    endif()

    set(CMAKE_C_COMPILER   "${ASTRA_GCC_PATH}/bin/gcc")
    set(CMAKE_CXX_COMPILER "${ASTRA_GCC_PATH}/bin/g++")
    set(CMAKE_LINKER       "${ASTRA_GCC_PATH}/bin/g++")

    # moc: передаём нужные дефайны (аналог QMAKE_MOC_FLAGS)
    set(CMAKE_AUTOMOC_MOC_OPTIONS
        "-D__GNUC__=10" "-D__GNUC_MINOR__=3" "-D__GNUC_PATCHLEVEL__=0")

    # include для самого gcc10
    include_directories(SYSTEM
        "${ASTRA_GCC_PATH}/include/c++/10.1.0"
        "${ASTRA_GCC_PATH}/include/c++/10.1.0/x86_64-pc-linux-gnu"
        "${ASTRA_GCC_PATH}/include/c++/10.1.0/backward"
        "${ASTRA_GCC_PATH}/include")
    link_directories("${ASTRA_GCC_PATH}/lib64" "${ASTRA_QT_PATH}/lib")
    message(STATUS "Astra/GCC10: ${ASTRA_GCC_PATH}, Qt: ${ASTRA_QT_PATH}")
endif()

# ============================================================
# Core-SDK install layout
#   Linux:   /opt/core/<version>/{include,lib,bin,lib/cmake/Core}
#   Windows: %LOCALAPPDATA%\Core\<version>\{include,lib,bin,lib\cmake\Core}
#   Cross-compile Windows from Linux/Docker: staging at ${CMAKE_BINARY_DIR}/applocal-staging
#
# Override (highest priority):
#   -DCORE_INSTALL_PREFIX=...
#   or  export CORE_INSTALL_PREFIX=...
# ============================================================
if(NOT DEFINED CORE_VERSION)
    message(FATAL_ERROR
        "CORE_VERSION is not set. It must be parsed from VERSION "
        "BEFORE include(cmake/Paths.cmake) in CMakeLists.txt.")
endif()

# Qt-подкаталог для разведения Qt5/Qt6
if(NOT DEFINED QT_VERSION_MAJOR)
    message(FATAL_ERROR "QT_VERSION_MAJOR must be set before include(cmake/Paths.cmake)")
endif()
set(CORE_INSTALL_QT_SUBDIR "qt${QT_VERSION_MAJOR}")

set(_CORE_INSTALL_PREFIX_USER_SET FALSE)
if(DEFINED CORE_INSTALL_PREFIX)
    set(_CORE_INSTALL_PREFIX_USER_SET TRUE)
elseif(DEFINED ENV{CORE_INSTALL_PREFIX} AND NOT "$ENV{CORE_INSTALL_PREFIX}" STREQUAL "")
    set(_CORE_INSTALL_PREFIX_USER_SET TRUE)
endif()

if(NOT DEFINED CORE_INSTALL_PREFIX
   AND DEFINED ENV{CORE_INSTALL_PREFIX}
   AND NOT "$ENV{CORE_INSTALL_PREFIX}" STREQUAL "")
    set(CORE_INSTALL_PREFIX "$ENV{CORE_INSTALL_PREFIX}"
        CACHE PATH "Core install prefix")
endif()

if(NOT DEFINED CORE_INSTALL_PREFIX)
    if(WIN32)
        if(DEFINED ENV{LOCALAPPDATA} AND NOT "$ENV{LOCALAPPDATA}" STREQUAL "")
            set(CORE_INSTALL_PREFIX "$ENV{LOCALAPPDATA}/Core/${CORE_INSTALL_QT_SUBDIR}/${CORE_VERSION}"
                CACHE PATH "Core install prefix")
        else()
            set(CORE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/applocal-staging/${CORE_INSTALL_QT_SUBDIR}"
                CACHE PATH "Core staging directory (cross-build)")
        endif()
    else()
        if(DEFINED ENV{HOME} AND NOT "$ENV{HOME}" STREQUAL "")
            set(CORE_INSTALL_PREFIX "$ENV{HOME}/.local/core/${CORE_INSTALL_QT_SUBDIR}/${CORE_VERSION}"
                CACHE PATH "Core install prefix")
        else()
            set(CORE_INSTALL_PREFIX "/opt/core/${CORE_INSTALL_QT_SUBDIR}/${CORE_VERSION}"
                CACHE PATH "Core install prefix")
        endif()
    endif()
endif()

# Install-relative subdirs (relative to CMAKE_INSTALL_PREFIX).
# Относительные пути = relocatable: CoreTargets.cmake будет корректно
# вычислять _IMPORT_PREFIX при переносе SDK в другой каталог.
set(CORE_INSTALL_INCLUDEDIR "include"         CACHE STRING "Include subdir (relative)")
set(CORE_INSTALL_LIBDIR     "lib"             CACHE STRING "Lib subdir (relative)")
set(CORE_INSTALL_BINDIR     "bin"             CACHE STRING "Bin subdir (relative)")
set(CORE_INSTALL_CMAKEDIR   "lib/cmake/Core"  CACHE STRING "CMake config subdir (relative)")

# `cmake --install` (без --prefix) должен попасть в CORE_INSTALL_PREFIX
# ── Sync CMAKE_INSTALL_PREFIX ← CORE_INSTALL_PREFIX ────────────────────
# Кросс-обёртки (i686-w64-mingw32-cmake) и некоторые arm-дистрибутивы
# задают CMAKE_INSTALL_PREFIX явно, поэтому INITIALIZED_TO_DEFAULT = FALSE.
# Тогда наш install() уходит не туда. Синхронизируем в двух случаях:
#   1. CMAKE_INSTALL_PREFIX остался дефолтным CMake → переопределяем.
#   2. Пользователь явно передал CORE_INSTALL_PREFIX (-D или env) и он
#      отличается от CMAKE_INSTALL_PREFIX → переопределяем (явный намеренный
#      override важнее кросс-toolchain sysroot).
# Если пользователь задал CMAKE_INSTALL_PREFIX, но НЕ задал CORE_INSTALL_PREFIX,
# его CMAKE_INSTALL_PREFIX уважается.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CORE_INSTALL_PREFIX}" CACHE PATH "" FORCE)
elseif(NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "${CORE_INSTALL_PREFIX}"
       AND _CORE_INSTALL_PREFIX_USER_SET)
    message(STATUS "Overriding CMAKE_INSTALL_PREFIX ('${CMAKE_INSTALL_PREFIX}') "
                   "with CORE_INSTALL_PREFIX ('${CORE_INSTALL_PREFIX}')")
    set(CMAKE_INSTALL_PREFIX "${CORE_INSTALL_PREFIX}" CACHE PATH "" FORCE)
endif()

# Абсолютный путь для POST_BUILD deploy-копий (аналог core-deploy.pri)
set(CORE_DEPLOY_LIBDIR "${CMAKE_INSTALL_PREFIX}/${CORE_INSTALL_LIBDIR}")

message(STATUS "Core Qt subdir: ${CORE_INSTALL_QT_SUBDIR}")
message(STATUS "Core install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  headers: ${CORE_INSTALL_INCLUDEDIR}")
message(STATUS "  libs:    ${CORE_INSTALL_LIBDIR}")
message(STATUS "  bins:    ${CORE_INSTALL_BINDIR}")
message(STATUS "  cmake:   ${CORE_INSTALL_CMAKEDIR}")
message(STATUS "  deploy:  ${CORE_DEPLOY_LIBDIR}")
