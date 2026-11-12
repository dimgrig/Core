QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += network serialport serialbus

TEMPLATE = lib
DEFINES += CORE_MODBUS_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/bindings"

HEADERS += \
    IModbusService.h \
    MockModbusClient.h \
    ModbusBase.h \
    ModbusClient.h \
    ModbusClientService.h \
    ModbusCsvImporter.h \
    ModbusDataConverter.h \
    ModbusMultiAddressServer.h \
    ModbusRegisterSerializer.h \
    ModbusScale.h \
    ModbusServer.h \
    ModbusSettings.h \
    core-modbus-lib-global.h \
    modbus_functions.h


SOURCES += \
    IModbusService.cpp \
    MockModbusClient.cpp \
    ModbusBase.cpp \
    ModbusClient.cpp \
    ModbusClientService.cpp \
    ModbusCsvImporter.cpp \
    ModbusDataConverter.cpp \
    ModbusMultiAddressServer.cpp \
    ModbusRegisterSerializer.cpp \
    ModbusScale.cpp \
    ModbusServer.cpp \
    ModbusSettings.cpp \
    modbus_functions.cpp

FORMS += \


CORE_LINK_LIBS = core-lib core-journal-lib
include(../core-build.pri)
VERSION = $$CORE_VERSION

# Install layout — паритет с CMake install (см. cmake/CoreLib.cmake).
unix:!macx {
    target.path   = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    headers.files = $$HEADERS
    headers.path  = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
}
win32|win64 {
    headers.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
    INSTALLS += headers

    contains(CONFIG, staticlib) {
        target_lib.files = $$DESTDIR/lib$${TARGET}.a
        target_lib.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
        INSTALLS += target_lib
    } else {
        target_bin.files = $$DESTDIR/$${TARGET}.dll
        target_bin.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_BINDIR
        target_lib.files = $$DESTDIR/lib$${TARGET}.a
        target_lib.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
        INSTALLS += target_bin target_lib
    }
}
headers.files =
for(h, HEADERS) {
    headers.files += $$_PRO_FILE_PWD_/$$h
}
INSTALLS += target headers
