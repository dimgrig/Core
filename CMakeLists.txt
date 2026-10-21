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

message(STATUS "Core deploy dir: ${CORE_DEPLOY_LIBDIR}")
message(STATUS "Core destination path: ${CORE_PLATFORM}/${CORE_COMPILER}/${CORE_PROCESSOR}/${CORE_BUILD_TYPE}")

# Паритет с qmake: CONFIG += release → -O2, а не -O3 (CMake default).
# -O3 агрессивнее инлайнит → меньше weak-символов → ABI-различия с PRO.
string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE     "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
