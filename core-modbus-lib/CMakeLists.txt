# ── Формируем список Qt-модулей динамически ──────────────────
set(_modbus_qt Core Network)
if(CORE_ENABLE_SERIAL)
    list(APPEND _modbus_qt SerialPort SerialBus)
endif()

core_add_library(core-modbus-lib
    QT ${_modbus_qt}
    DEPENDS core-lib core-journal-lib
)

# ── Базовые исходники (всегда компилируются) ─────────────────
target_sources(core-modbus-lib PRIVATE
    IModbusService.cpp
    MockModbusClient.cpp
    ModbusBase.cpp
    ModbusClient.cpp
    ModbusClientService.cpp
    ModbusDataConverter.cpp
    ModbusRegisterSerializer.cpp
    ModbusServer.cpp
    ModbusSettings.cpp
    modbus_functions.cpp

    IModbusService.h
    MockModbusClient.h
    ModbusBase.h
    ModbusClient.h
    ModbusClientService.h
    ModbusDataConverter.h
    ModbusRegisterSerializer.h
    ModbusServer.h
    ModbusSettings.h
    core-modbus-lib-global.h
    modbus_functions.h

    ModbusCsvImporter.h ModbusCsvImporter.cpp
    ModbusScale.h ModbusScale.cpp

    WasmModbusStubs.h
)

# ── Serial-специфичные файлы (только если CORE_ENABLE_SERIAL) ─
if(CORE_ENABLE_SERIAL)
    target_sources(core-modbus-lib PRIVATE
        ModbusMultiAddressServer.cpp
        ModbusMultiAddressServer.h
    )
    target_compile_definitions(core-modbus-lib PUBLIC CORE_ENABLE_SERIAL)
else()
    # Для Wasm: определяем макрос, чтобы заголовки не включали QModbusDevice/QModbusClient
    target_compile_definitions(core-modbus-lib PUBLIC CORE_DISABLE_SERIAL)
endif()

# ── Qt6 Core5Compat для QTextCodec ───────────────────────────
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(core-modbus-lib PRIVATE Qt6::Core5Compat)
endif()
