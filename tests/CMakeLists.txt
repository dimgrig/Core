# ──────────────────────────────────────────────────────────────────────
# tests/CMakeLists.txt — QtTest-приложение core-tests
# Аналог tests.pro (TEMPLATE = app)
# ──────────────────────────────────────────────────────────────────────
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Sql)

add_executable(core-tests
    main.cpp
    TestAuth.cpp
    TestJournal.cpp
    TestModbusIntegration.cpp
    TestAuth.h
    TestJournal.h
    TestModbusIntegration.h
    resources/resources.qrc
)

# Qt Creator-friendly: moc/uic/rcc включены через свойства target
set_target_properties(core-tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(core-tests PRIVATE
    core-lib
    core-io-lib
    core-journal-lib
    core-modbus-lib
    core-mvc-lib
    core-utils-lib
    core-widgets-lib
    core-plot-lib
    muParser
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
)

target_include_directories(core-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# ──────────────────────────────────────────────────────────────────────
# Запуск после сборки — аналог QMAKE_POST_LINK в tests.pro
# (в Docker/CI удобно, локально тоже — сразу видно падения)
# ──────────────────────────────────────────────────────────────────────
if(NOT WIN32)
    add_custom_command(TARGET core-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env
            QT_QPA_PLATFORM=offscreen
            LD_LIBRARY_PATH=${CORE_DEPLOY_LIBDIR}:$ENV{LD_LIBRARY_PATH}
            $<TARGET_FILE:core-tests>
        COMMENT "Running core-tests (offscreen)")
endif()

# ──────────────────────────────────────────────────────────────────────
# Регистрация в CTest (порядок ВАЖЕН: сначала add_test, потом properties)
# ──────────────────────────────────────────────────────────────────────
add_test(NAME core-tests COMMAND core-tests)

set_tests_properties(core-tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LD_LIBRARY_PATH=${CORE_DEPLOY_LIBDIR}"
    TIMEOUT 300
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
