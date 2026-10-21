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
#     VERSION   2.1.1                # опционально (дефолт 1.0.0)
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
        set(ARG_VERSION "1.0.0")
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
        $<INSTALL_INTERFACE:include>)
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

    # ── Deploy (аналог QMAKE_POST_LINK из core-deploy.pri) ──
    if(NOT CORE_NO_DEPLOY)
        if(WIN32)
            add_custom_command(TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CORE_DEPLOY_LIBDIR}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${name}>        "${CORE_DEPLOY_LIBDIR}/"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_LINKER_FILE:${name}> "${CORE_DEPLOY_LIBDIR}/"
                COMMENT "Deploy ${name} → ${CORE_DEPLOY_LIBDIR}")
        else()
            # libtool/qmake создают промежуточный .so.MAJOR.MINOR → .so.VERSION,
            # CMake — нет. Для паритета с PRO-сборкой добавляем его вручную.
            # Например: VERSION 1.0.0 → SOVERSION 1, промежуточный .so.1.0
            #            VERSION 2.1.1 → SOVERSION 2, промежуточный .so.2.1
            #            VERSION 1.9.0 → SOVERSION 1, промежуточный .so.1.9
            set(_intermediate_symlink_cmds "")
            if(ARG_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.")
                set(_mm "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
                # Создаём, только если MAJOR.MINOR отличается и от SOVERSION,
                # и от полной VERSION (иначе — дубликат).
                if(NOT _mm STREQUAL _soversion AND NOT _mm STREQUAL ARG_VERSION)
                    list(APPEND _intermediate_symlink_cmds
                        COMMAND ${CMAKE_COMMAND} -E rm -f
                            "${CORE_DEPLOY_LIBDIR}/lib${name}.so.${_mm}"
                        COMMAND ${CMAKE_COMMAND} -E create_symlink
                            "lib${name}.so.${ARG_VERSION}"
                            "${CORE_DEPLOY_LIBDIR}/lib${name}.so.${_mm}")
                endif()
            endif()

            add_custom_command(TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CORE_DEPLOY_LIBDIR}"
                COMMAND sh -c "cp -af '${CMAKE_CURRENT_BINARY_DIR}/lib${name}.so'* '${CORE_DEPLOY_LIBDIR}/'"
                ${_intermediate_symlink_cmds}
                COMMENT "Deploy ${name} → ${CORE_DEPLOY_LIBDIR}")
        endif()
    endif()

    # ── Install ──
    install(TARGETS ${name}
        LIBRARY DESTINATION "${CORE_DEPLOY_LIBDIR}"
        RUNTIME DESTINATION "${CORE_DEPLOY_LIBDIR}"
        ARCHIVE DESTINATION "${CORE_DEPLOY_LIBDIR}")
endfunction()
