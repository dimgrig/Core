QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network serialport serialbus mqtt

TEMPLATE = lib
DEFINES += CORE_IO_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

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

CORE_LINK_LIBS = core-lib core-journal-lib core-modbus-lib
include(../core-build.pri)
VERSION = $$CORE_VERSION

# Install layout — паритет с CMake install (см. cmake/CoreLib.cmake).
unix:!macx {
    target.path   = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    headers.files = $$HEADERS
    headers.path  = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
}
win32|win64 {
    # На mingw .dll и .a попадают в lib — qmake INSTALLS не разделяет RUNTIME/ARCHIVE.
    # Для разделения (bin/ для .dll) используется CMake-путь.
    target.path   = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
    headers.files = $$HEADERS
    headers.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
}
headers.files =
for(h, HEADERS) {
    headers.files += $$_PRO_FILE_PWD_/$$h
}
INSTALLS += target headers
