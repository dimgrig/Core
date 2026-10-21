core_add_library(core-modbus-lib
    QT Core PrintSupport Network SerialPort SerialBus
    DEPENDS core-lib core-journal-lib
    INCLUDES bindings
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
    bindings/AdvancedClickablePixmapLabelBinding.cpp
    bindings/BindingFactory.cpp
    bindings/BindingManager.cpp
    bindings/BitAwareBindingManager.cpp
    bindings/CheckBoxBinding.cpp
    bindings/ClickablePixmapLabelBinding.cpp
    bindings/ComboBoxBinding.cpp
    bindings/DoubleSpinBoxBinding.cpp
    bindings/LabelBinding.cpp
    bindings/LineEditBinding.cpp
    bindings/PixmapLabelBinding.cpp
    bindings/RegisterBinding.cpp
    bindings/SpinBoxBinding.cpp
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
    bindings/AdvancedClickablePixmapLabelBinding.h
    bindings/BindingFactory.h
    bindings/BindingManager.h
    bindings/BitAwareBindingManager.h
    bindings/CheckBoxBinding.h
    bindings/ClickablePixmapLabelBinding.h
    bindings/ComboBoxBinding.h
    bindings/DoubleSpinBoxBinding.h
    bindings/LabelBinding.h
    bindings/LineEditBinding.h
    bindings/PixmapLabelBinding.h
    bindings/RegisterBinding.h
    bindings/SpinBoxBinding.h
    core-modbus-lib-global.h
    modbus_functions.h

    uis/modbus_settings.ui
)

target_include_directories(core-modbus-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings)

