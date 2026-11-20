cmake_minimum_required(VERSION 3.16)
project(test-smtp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ── Qt6 ──────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Network)

# ── Core SDK ─────────────────────────────────────────────────────────
# CORE_SDK_ROOT можно передать через -DCORE_SDK_ROOT=/path/to/core
# или установить переменную окружения
if(NOT DEFINED CORE_SDK_ROOT)
    if(DEFINED ENV{CORE_SDK_ROOT})
        set(CORE_SDK_ROOT "$ENV{CORE_SDK_ROOT}")
    else()
        # Дефолтный путь для твоей системы
        set(CORE_SDK_ROOT "/home/dima/deploy/core/latest/linux-qt6")
    endif()
endif()

if(NOT EXISTS "${CORE_SDK_ROOT}/lib/cmake/Core/CoreConfig.cmake")
    message(FATAL_ERROR "Core SDK not found at ${CORE_SDK_ROOT}")
endif()

set(Core_DIR "${CORE_SDK_ROOT}/lib/cmake/Core" CACHE PATH "" FORCE)
find_package(Core CONFIG REQUIRED)

message(STATUS "Core SDK: ${CORE_SDK_ROOT}")
message(STATUS "Qt6: ${Qt6_VERSION}")

# ── Executable ───────────────────────────────────────────────────────
add_executable(test-smtp main.cpp)

target_link_libraries(test-smtp PRIVATE
    Core::core-io-lib
    Qt6::Core
    Qt6::Network
)

# RPATH для запуска из build-директории
set_target_properties(test-smtp PROPERTIES
    INSTALL_RPATH "${CORE_SDK_ROOT}/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)