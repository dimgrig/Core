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

    # ── Создаём target без sources (нужно для target_sources-паттерна) ──
    # add_library без sources поддерживается в CMake ≥ 3.11.
    add_library(${name} SHARED)
    add_library(Core::${name} ALIAS ${name})

    # ── Если sources переданы напрямую — добавляем ──
    # (QtWebApp и др. могут передавать через GLOB, это допустимо)
    if(ARG_SOURCES OR ARG_HEADERS OR ARG_FORMS OR ARG_RESOURCES)
        target_sources(${name} PRIVATE
            ${ARG_SOURCES} ${ARG_HEADERS} ${ARG_FORMS} ${ARG_RESOURCES})
    endif()

    # ── Экспортный макрос: CORE_LIB_LIBRARY, CORE_IO_LIB_LIBRARY, ... ──
    string(TOUPPER "${name}" _upper)
    string(REPLACE "-" "_" _upper "${_upper}")
    target_compile_definitions(${name} PRIVATE ${_upper}_LIBRARY)

    # ── Дефолты из core-build.pri ──
    target_compile_definitions(${name} PRIVATE NOCRYPT NOUSER)
    target_compile_options(${name} PRIVATE -Wno-deprecated)

    # ── Версия / SONAME ──
    if(NOT ARG_VERSION)
        if(PROJECT_VERSION)
            set(ARG_VERSION "${PROJECT_VERSION}")
        else()
            message(FATAL_ERROR
                "core_add_library(${name}): VERSION не передан и PROJECT_VERSION пуст. "
                "Убедитесь, что в корневом CMakeLists.txt есть "
                "project(Core VERSION x.y.z ...)")
        endif()
    endif()
    if(ARG_VERSION MATCHES "^([0-9]+)")
        set(_soversion "${CMAKE_MATCH_1}")
    else()
        set(_soversion "1")
    endif()

    # ── Свойства target (включая AUTOMOC/AUTOUIC/AUTORCC) ──
    set_target_properties(${name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
        VERSION   ${ARG_VERSION}
        SOVERSION ${_soversion}

        # Qt-кодогенерация:
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        # .ui-файлы обычно лежат в подпапке uis/, а cpp — в корне.
        # uic ищет `<путь>/ui_<basename>.h` → `<путь>/<basename>.ui`,
        # поэтому добавляем и корень, и uis/.
        AUTOUIC_SEARCH_PATHS
            "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/uis"
    )

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

    # ── RPATH ──
    # Паритет с qmake: PRO-RPATH пуст, .so грузятся через ldconfig/LD_LIBRARY_PATH.
    set_target_properties(${name} PROPERTIES
        INSTALL_RPATH       ""
        BUILD_RPATH         ""
        BUILD_WITH_INSTALL_RPATH FALSE
        SKIP_BUILD_RPATH    TRUE)

    # ── Install: files to CORE_INSTALL_* ──
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
