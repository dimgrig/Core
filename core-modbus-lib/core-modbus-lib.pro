QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport network serialport serialbus

TEMPLATE = lib
DEFINES += CORE_MODBUS_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/bindings"

CORE_LINK_LIBS = core-lib core-journal-lib
include(../core-build.pri)
VERSION = $$CORE_VERSION

HEADERS += \
    IModbusService.h \
    MockModbusClient.h \
    ModbusBase.h \
    ModbusClient.h \
    ModbusClientService.h \
    ModbusDataConverter.h \
    ModbusMultiAddressServer.h \
    ModbusRegisterSerializer.h \
    ModbusServer.h \
    ModbusSettings.h \
    bindings/AdvancedClickablePixmapLabelBinding.h \
    bindings/BindingFactory.h \
    bindings/BindingManager.h \
    bindings/BitAwareBindingManager.h \
    bindings/CheckBoxBinding.h \
    bindings/ClickablePixmapLabelBinding.h \
    bindings/ComboBoxBinding.h \
    bindings/DoubleSpinBoxBinding.h \
    bindings/LabelBinding.h \
    bindings/LineEditBinding.h \
    bindings/PixmapLabelBinding.h \
    bindings/RegisterBinding.h \
    bindings/SpinBoxBinding.h \
    core-modbus-lib-global.h \
    modbus_functions.h

SOURCES += \
    IModbusService.cpp \
    MockModbusClient.cpp \
    ModbusBase.cpp \
    ModbusClient.cpp \
    ModbusClientService.cpp \
    ModbusDataConverter.cpp \
    ModbusMultiAddressServer.cpp \
    ModbusRegisterSerializer.cpp \
    ModbusServer.cpp \
    ModbusSettings.cpp \
    bindings/AdvancedClickablePixmapLabelBinding.cpp \
    bindings/BindingFactory.cpp \
    bindings/BindingManager.cpp \
    bindings/BitAwareBindingManager.cpp \
    bindings/CheckBoxBinding.cpp \
    bindings/ClickablePixmapLabelBinding.cpp \
    bindings/ComboBoxBinding.cpp \
    bindings/DoubleSpinBoxBinding.cpp \
    bindings/LabelBinding.cpp \
    bindings/LineEditBinding.cpp \
    bindings/PixmapLabelBinding.cpp \
    bindings/RegisterBinding.cpp \
    bindings/SpinBoxBinding.cpp \
    modbus_functions.cpp

FORMS += \
    uis/modbus_settings.ui

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
INSTALLS += target headers
