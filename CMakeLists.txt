cmake_minimum_required(VERSION 3.18)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CORE_VERSION LIMIT_COUNT 1)
string(STRIP "${CORE_VERSION}" CORE_VERSION)
string(REGEX MATCH "^[0-9]+" CORE_VERSION_MAJOR "${CORE_VERSION}")
if(NOT CORE_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must be X.Y.Z, got '${CORE_VERSION}'")
endif()

project(Core VERSION ${CORE_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Цветной вывод
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Qt detection (до того, как понадобится QT_VERSION_MAJOR)
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
#message(STATUS "Qt version: ${QT_VERSION_MAJOR}")
# Qt5
find_package(Qt5 REQUIRED COMPONENTS Core)
set(QT_VERSION_MAJOR 5)
message(STATUS "Qt version: ${Qt5Core_VERSION} (forced Qt5)")

include(cmake/Platform.cmake)
include(cmake/Paths.cmake)
include(cmake/CoreLib.cmake)

# ---- Внешние библиотеки (сначала, т.к. от них зависят core-*) ----
add_subdirectory(muParser)
add_subdirectory(qcustomplot)
add_subdirectory(QtWebApp)

# ---- Core-библиотеки в порядке зависимостей ----
add_subdirectory(core-lib)
add_subdirectory(core-journal-lib)
add_subdirectory(core-widgets-lib)
add_subdirectory(core-modbus-lib)
add_subdirectory(core-io-lib)
add_subdirectory(core-utils-lib)
add_subdirectory(core-plot-lib)
add_subdirectory(core-mvc-lib)

# ---- Тесты (по умолчанию ВКЛ, отключается -DBUILD_TESTING=OFF) ----
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# ============================================================
# Установка Core-SDK: пакетный конфиг для find_package(Core)
# ============================================================
include(CMakePackageConfigHelpers)

# ────────────────────────────────────────────────────────────
# RPATH для Core shared libraries
# ────────────────────────────────────────────────────────────
# DT_RUNPATH (дефолт современного ld) не транзитивен: если
# libcore-widgets-lib.so.1 зависит от libcore-journal-lib.so.1,
# RUNPATH исполнителя её не найдёт. Устанавливаем $ORIGIN,
# чтобы каждая .so искала соседей в своём же каталоге.
if(UNIX AND NOT APPLE)
    set(_core_rpath "$ORIGIN")
elseif(APPLE)
    set(_core_rpath "@loader_path")
else()
    set(_core_rpath "")
endif()

if(NOT "${_core_rpath}" STREQUAL "")
    foreach(_core_lib
            core-lib
            core-journal-lib
            core-widgets-lib
            core-modbus-lib
            core-io-lib
            core-utils-lib
            core-plot-lib
            core-mvc-lib
            muParser
            qcustomplot
            QtWebApp)
        if(TARGET ${_core_lib})
            set_target_properties(${_core_lib} PROPERTIES
                INSTALL_RPATH "${_core_rpath}"
                INSTALL_RPATH_USE_LINK_PATH TRUE
                BUILD_WITH_INSTALL_RPATH OFF)
        endif()
    endforeach()
endif()

install(EXPORT CoreTargets
    FILE CoreTargets.cmake
    NAMESPACE Core::
    DESTINATION "${CORE_INSTALL_CMAKEDIR}")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CoreConfig.cmake"
    INSTALL_DESTINATION "${CORE_INSTALL_CMAKEDIR}")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CoreConfigVersion.cmake"
    VERSION ${CORE_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CoreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CoreConfigVersion.cmake"
    DESTINATION "${CORE_INSTALL_CMAKEDIR}")

message(STATUS "Core deploy dir: ${CORE_DEPLOY_LIBDIR}")
message(STATUS "Core destination path: ${CORE_PLATFORM}/${CORE_COMPILER}/${CORE_PROCESSOR}/${CORE_BUILD_TYPE}")

# Паритет с qmake: CONFIG += release → -O2, а не -O3 (CMake default).
# -O3 агрессивнее инлайнит → меньше weak-символов → ABI-различия с PRO.
string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE     "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# ────────────────────────────────────────────────────────────
# latest → <version> — sibling symlink (Windows: junction)
# ────────────────────────────────────────────────────────────
# Даёт потребителям (MODBUS-CLIENT/SERVER, Saks) единый путь
# <base>/latest без привязки к версии. Идемпотентно:
#   - Linux:  ln -sfn (заменяет, если есть)
#   - Windows: rmdir + mklink /J (junction без elevation)
install(CODE [[
    get_filename_component(_latest_base "${CMAKE_INSTALL_PREFIX}" DIRECTORY)
    set(_latest "${_latest_base}/latest")
    if(NOT "${_latest}" STREQUAL "${CMAKE_INSTALL_PREFIX}")
        message(STATUS "Creating ${_latest} -> ${CMAKE_INSTALL_PREFIX}")
        if(WIN32)
            execute_process(COMMAND cmd /c
                "if exist \"${_latest}\" rmdir \"${_latest}\" 2>nul & mklink /J \"${_latest}\" \"${CMAKE_INSTALL_PREFIX}\""
                RESULT_VARIABLE _rc)
        else()
            execute_process(COMMAND ln -sfn "${CMAKE_INSTALL_PREFIX}" "${_latest}"
                RESULT_VARIABLE _rc)
        endif()
        if(NOT _rc EQUAL 0)
            message(WARNING "Failed to create ${_latest} (rc=${_rc})")
        endif()
    endif()
]])
