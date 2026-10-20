QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network serialport serialbus mqtt

TEMPLATE = lib
DEFINES += CORE_IO_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

CORE_LINK_LIBS = core-lib core-journal-lib core-modbus-lib
include(../core-build.pri)

unix { target.path = /usr/lib/ }
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ClientHelpers.h \
    HTTPClient.h \
    MQTTClient.h \
    MockMQTTClient.h \
    ResourceManager.h \
    SCPISettings.h \
    ScpiDevice.h \
    ScpiTransport.h \
    SerialClient.h \
    TCPClient.h \
    TCPServer.h \
    UDPClient.h \
    UDPServer.h \
    core-io-lib-global.h \
    scpi_functions.h

SOURCES += \
    HTTPClient.cpp \
    MQTTClient.cpp \
    MockMQTTClient.cpp \
    ResourceManager.cpp \
    SCPISettings.cpp \
    ScpiDevice.cpp \
    ScpiTransport.cpp \
    SerialClient.cpp \
    TCPClient.cpp \
    TCPServer.cpp \
    UDPClient.cpp \
    UDPServer.cpp \
    scpi_functions.cpp

FORMS += \
    uis/scpi_settings.ui
