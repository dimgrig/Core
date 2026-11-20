# ── Формируем список Qt-модулей динамически ──────────────────
set(_io_qt Core Network)
if(CORE_ENABLE_SERIAL)
    list(APPEND _io_qt SerialPort)
endif()

# Mqtt - опциональный модуль
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Mqtt)
if(TARGET Qt${QT_VERSION_MAJOR}::Mqtt)
    list(APPEND _io_qt Mqtt)
endif()

core_add_library(core-io-lib
    QT ${_io_qt}
    DEPENDS core-lib core-journal-lib
)

# ── Базовые исходники ────────────────────────────────────────
target_sources(core-io-lib PRIVATE
    HTTPClient.cpp
    ResourceManager.cpp
    SmtpClient.cpp
    TCPClient.cpp
    TCPServer.cpp
    UDPClient.cpp
    UDPServer.cpp
    scpi_functions.cpp

    ClientHelpers.h
    HTTPClient.h
    ResourceManager.h
    SmtpClient.h
    TCPClient.h
    TCPServer.h
    UDPClient.h
    UDPServer.h
    core-io-lib-global.h
    scpi_functions.h
)

# ── Serial-специфичные файлы ─────────────────────────────────
if(CORE_ENABLE_SERIAL)
    target_sources(core-io-lib PRIVATE
        SerialClient.cpp
        SCPISettings.cpp
        ScpiDevice.cpp
        ScpiTransport.cpp

        SerialClient.h
        SCPISettings.h
        ScpiDevice.h
        ScpiTransport.h
    )
    target_compile_definitions(core-io-lib PUBLIC CORE_ENABLE_SERIAL)
else()
    # Для Wasm: определяем макрос, чтобы заголовки не включали QSerialPort
    target_compile_definitions(core-io-lib PUBLIC CORE_DISABLE_SERIAL)
endif()

# ── MQTT-специфичные файлы ───────────────────────────────────
if(TARGET Qt${QT_VERSION_MAJOR}::Mqtt)
    target_sources(core-io-lib PRIVATE
        MQTTClient.cpp
        MockMQTTClient.cpp
        MQTTClient.h
        MockMQTTClient.h
    )
    target_compile_definitions(core-io-lib PUBLIC CORE_ENABLE_MQTT)
endif()
