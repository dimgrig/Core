include_guard(GLOBAL)

# core_add_library(<name>
#     SOURCES   a.cpp b.cpp          # опционально — для старых CMakeLists
#     HEADERS   a.h b.h              # опционально
#     FORMS     x.ui                 # опционально
#     RESOURCES r.qrc                # опционально
#     QT        Core Widgets         # модули Qt (QT += в .pro)
#     DEPENDS   core-lib             # PUBLIC-зависимости
#     DEFINES   FOO BAR              # PRIVATE defines
#     INCLUDES  some/dir             # PRIVATE include-пути
#     VERSION   2.1.1                # опционально (дефолт = PROJECT_VERSION)
# )
#
# Файлы можно не передавать через SOURCES/HEADERS, а добавлять
# через target_sources(<name> PRIVATE ...) — это Qt Creator-friendly
# паттерн: он дописывает новые классы прямо в target_sources().
function(core_add_library name)
    cmake_parse_arguments(ARG "" "VERSION"
        "SOURCES;HEADERS;FORMS;RESOURCES;QT;DEPENDS;DEFINES;INCLUDES" ${ARGN})

    # ── 1. Выбор типа библиотеки: STATIC для Wasm, SHARED для десктопа ──
    if(CORE_IS_WASM)
        add_library(${name} STATIC)
    else()
        add_library(${name} SHARED)
    endif()

    add_library(Core::${name} ALIAS ${name})

    # ── Если sources переданы напрямую — добавляем ──
    if(ARG_SOURCES OR ARG_HEADERS OR ARG_FORMS OR ARG_RESOURCES)
        target_sources(${name} PRIVATE
            ${ARG_SOURCES} ${ARG_HEADERS} ${ARG_FORMS} ${ARG_RESOURCES})
    endif()

    # ── Экспортный макрос ──
    string(TOUPPER "${name}" _upper)
    string(REPLACE "-" "_" _upper "${_upper}")
    target_compile_definitions(${name} PRIVATE ${_upper}_LIBRARY)

    # ── Дефолты ──
    target_compile_definitions(${name} PRIVATE NOCRYPT NOUSER)
    target_compile_options(${name} PRIVATE -Wno-deprecated)

    # ── Версия / SONAME (только для SHARED библиотек) ──
    if(NOT ARG_VERSION)
        if(PROJECT_VERSION)
            set(ARG_VERSION "${PROJECT_VERSION}")
        else()
            message(FATAL_ERROR "core_add_library(${name}): VERSION не передан и PROJECT_VERSION пуст.")
        endif()
    endif()

    if(NOT CORE_IS_WASM AND ARG_VERSION MATCHES "^([0-9]+)")
        set(_soversion "${CMAKE_MATCH_1}")
    else()
        set(_soversion "1")
    endif()

    # ── 2. Свойства target: для Wasm скрываем видимость и убираем SOVERSION ──
    if(CORE_IS_WASM)
        set_target_properties(${name} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            POSITION_INDEPENDENT_CODE ON
            # ВАЖНО для Wasm: скрываем все символы по умолчанию, экспортируем только Q_DECL_EXPORT
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
            AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/uis"
        )
    else()
        set_target_properties(${name} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            POSITION_INDEPENDENT_CODE ON
            CXX_VISIBILITY_PRESET default
            VISIBILITY_INLINES_HIDDEN OFF
            VERSION   ${ARG_VERSION}
            SOVERSION ${_soversion}
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
            AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/uis"
        )
    endif()

    # ── Include-пути ──
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}>)
    if(ARG_INCLUDES)
        target_include_directories(${name} PRIVATE ${ARG_INCLUDES})
    endif()

    # ── Qt-модули ──
    if(ARG_QT)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${ARG_QT})
        foreach(_m ${ARG_QT})
            target_link_libraries(${name} PUBLIC Qt${QT_VERSION_MAJOR}::${_m})
        endforeach()
    endif()

    # ── Зависимости ──
    if(ARG_DEPENDS)
        target_link_libraries(${name} PUBLIC ${ARG_DEPENDS})
    endif()

    if(ARG_DEFINES)
        target_compile_definitions(${name} PRIVATE ${ARG_DEFINES})
    endif()

    # ── RPATH (только для SHARED) ──
    if(NOT CORE_IS_WASM)
        set_target_properties(${name} PROPERTIES
            INSTALL_RPATH       ""
            BUILD_RPATH         ""
            BUILD_WITH_INSTALL_RPATH FALSE
            SKIP_BUILD_RPATH    TRUE)
    endif()

    # ── Install: files to CORE_INSTALL_* ──
    # ARCHIVE DESTINATION корректно обработает .a файлы для Wasm
    install(TARGETS ${name}
        EXPORT CoreTargets
        RUNTIME DESTINATION "${CORE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CORE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CORE_INSTALL_LIBDIR}")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION "${CORE_INSTALL_INCLUDEDIR}/${name}"
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
            PATTERN "*.inl")
endfunction()
