core_add_library(core-modbus-lib
    QT Core Network SerialPort SerialBus
    DEPENDS core-lib core-journal-lib
)

target_sources(core-modbus-lib PRIVATE
    IModbusService.cpp
    MockModbusClient.cpp
    ModbusBase.cpp
    ModbusClient.cpp
    ModbusClientService.cpp
    ModbusDataConverter.cpp
    ModbusMultiAddressServer.cpp
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
    ModbusMultiAddressServer.h
    ModbusRegisterSerializer.h
    ModbusServer.h
    ModbusSettings.h
    core-modbus-lib-global.h
    modbus_functions.h

    ModbusCsvImporter.h ModbusCsvImporter.cpp
    ModbusScale.h ModbusScale.cpp
)

#target_include_directories(core-modbus-lib PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bindings>
#    $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}/core-modbus-lib/bindings>
#)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(core-modbus-lib PRIVATE Qt6::Core5Compat) #QTextCodec
endif()

