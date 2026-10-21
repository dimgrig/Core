core_add_library(core-io-lib
    QT Core Widgets Network SerialPort SerialBus Mqtt
    DEPENDS core-lib core-journal-lib core-modbus-lib
)

target_sources(core-io-lib PRIVATE
    HTTPClient.cpp
    MQTTClient.cpp
    MockMQTTClient.cpp
    ResourceManager.cpp
    SCPISettings.cpp
    ScpiDevice.cpp
    ScpiTransport.cpp
    SerialClient.cpp
    TCPClient.cpp
    TCPServer.cpp
    UDPClient.cpp
    UDPServer.cpp
    scpi_functions.cpp

    ClientHelpers.h
    HTTPClient.h
    MQTTClient.h
    MockMQTTClient.h
    ResourceManager.h
    SCPISettings.h
    ScpiDevice.h
    ScpiTransport.h
    SerialClient.h
    TCPClient.h
    TCPServer.h
    UDPClient.h
    UDPServer.h
    core-io-lib-global.h
    scpi_functions.h

    uis/scpi_settings.ui
)

