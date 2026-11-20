# ──────────────────────────────────────────────────────────────────────
# tests/CMakeLists.txt — QtTest-приложение core-tests
# Аналог tests.pro (TEMPLATE = app)
# ──────────────────────────────────────────────────────────────────────
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Sql)

set(_core_test_sources
    main.cpp
    TestAuth.cpp
    TestJournal.cpp
    TestSmtpClient.cpp

    TestAuth.h
    TestJournal.h
    TestSmtpClient.h

    resources/resources.qrc
)

# Интеграционные тесты Modbus требуют реального QtSerialBus/QtNetwork TCP стека,
# который в Wasm отсутствует (замокан). Собираем их только для десктопа.
if(NOT CORE_IS_WASM)
    list(APPEND _core_test_sources
        TestModbusIntegration.cpp
        TestModbusIntegration.h
    )
endif()

add_executable(core-tests ${_core_test_sources})

# При кросс-компиляции из Linux под Windows хост-сборка rcc умеет zstd,
# а целевая сборка Qt для Windows — нет. Чтобы rcc не сжимал ресурсы
# zstd'ом, явно отключаем это для тестового таргета.
if(WIN32 AND CMAKE_CROSSCOMPILING)
    set_target_properties(core-tests PROPERTIES
        AUTORCC_OPTIONS "--no-zstd")
endif()

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
    set(_core_test_lib_dirs
        "$<TARGET_FILE_DIR:core-lib>"
        "$<TARGET_FILE_DIR:core-journal-lib>"
        "$<TARGET_FILE_DIR:core-modbus-lib>"
        "$<TARGET_FILE_DIR:core-io-lib>"
        "$<TARGET_FILE_DIR:core-mvc-lib>"
        "$<TARGET_FILE_DIR:core-utils-lib>"
        "$<TARGET_FILE_DIR:core-widgets-lib>"
        "$<TARGET_FILE_DIR:core-plot-lib>"
        "$<TARGET_FILE_DIR:qcustomplot>"
        "$<TARGET_FILE_DIR:QtWebApp>"
    )
    list(JOIN _core_test_lib_dirs ":" _core_test_ld_path)

    add_custom_command(TARGET core-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env
            "QT_QPA_PLATFORM=offscreen"
            "LD_LIBRARY_PATH=${_core_test_ld_path}:$ENV{LD_LIBRARY_PATH}"
            --
            $<TARGET_FILE:core-tests>
        COMMENT "Running core-tests (offscreen, build tree)")
endif()
