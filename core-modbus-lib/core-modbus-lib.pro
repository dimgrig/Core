QT       += core network serialport serialbus
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport #webenginewidgets

TEMPLATE = lib
DEFINES += CORE_MODBUS_LIB_LIBRARY

CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-core-version.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-core-version.pri)
include(../qmake-destination-path.pri)

exists(/etc/astra_version) {
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-journal-lib

    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-journal-lib
    }

    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-journal-lib
    }
    win64 {
        DEFINES += win64
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-journal-lib
    }
}

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

DEFINES += ENABLE_CONVERSION

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/bindings"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-journal-lib"

BUILD_DIR = $$PWD/../../build/$${TARGET}-build/
DESTDIR = $$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.ui

# Default rules for deployment.
unix {
    target.path = /usr/lib/
}
!isEmpty(target.path): INSTALLS += target

unix {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-modbus-lib.so)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-modbus-lib.so.1)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-modbus-lib.so.1.0)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-modbus-lib.so.1.0.0)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
}

win32 {
#    # Используем относительные пути от текущего каталога
#    SOURCE_DLL_RELATIVE = ../core-modbus-lib-binaries/windows/gcc/x86/debug/core-modbus-lib.dll
#    SOURCE_LIB_RELATIVE = ../core-modbus-lib-binaries/windows/gcc/x86/debug/libcore-modbus-lib.a
#    DEST_RELATIVE = ../../../QtCreatorProjects/libs

#    # Получаем абсолютные пути
#    SOURCE_DLL_ABS = $$absolute_path($$SOURCE_DLL_RELATIVE, $$PWD)
#    SOURCE_LIB_ABS = $$absolute_path($$SOURCE_LIB_RELATIVE, $$PWD)
#    DEST_ABS = $$absolute_path($$DEST_RELATIVE, $$PWD)

#    # Преобразуем в системный формат
#    SOURCE_DLL_WIN = $$system_path($$SOURCE_DLL_ABS)
#    SOURCE_LIB_WIN = $$system_path($$SOURCE_LIB_ABS)
#    DEST_WIN = $$system_path($$DEST_ABS)

#    QMAKE_POST_LINK += $$QMAKE_COPY \"$$SOURCE_DLL_WIN\" \"$$DEST_WIN\"
#    QMAKE_POST_LINK += $$QMAKE_COPY \"$$SOURCE_LIB_WIN\" \"$$DEST_WIN\"

    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-modbus-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-modbus-lib.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}
win64 {
#    # Используем относительные пути от текущего каталога
#    SOURCE_DLL_RELATIVE = ../core-modbus-lib-binaries/windows/gcc/x86/debug/core-modbus-lib.dll
#    SOURCE_LIB_RELATIVE = ../core-modbus-lib-binaries/windows/gcc/x86/debug/libcore-modbus-lib.a
#    DEST_RELATIVE = ../../../QtCreatorProjects/libs

#    # Получаем абсолютные пути
#    SOURCE_DLL_ABS = $$absolute_path($$SOURCE_DLL_RELATIVE, $$PWD)
#    SOURCE_LIB_ABS = $$absolute_path($$SOURCE_LIB_RELATIVE, $$PWD)
#    DEST_ABS = $$absolute_path($$DEST_RELATIVE, $$PWD)

#    # Преобразуем в системный формат
#    SOURCE_DLL_WIN = $$system_path($$SOURCE_DLL_ABS)
#    SOURCE_LIB_WIN = $$system_path($$SOURCE_LIB_ABS)
#    DEST_WIN = $$system_path($$DEST_ABS)

#    QMAKE_POST_LINK += $$QMAKE_COPY \"$$SOURCE_DLL_WIN\" \"$$DEST_WIN\"
#    QMAKE_POST_LINK += $$QMAKE_COPY \"$$SOURCE_LIB_WIN\" \"$$DEST_WIN\"

    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-modbus-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-modbus-lib.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t))

HEADERS += \
    IModbusService.h \
    ModbusBase.h \
    ModbusClient.h \
    ModbusClientService.h \
    ModbusDataConverter.h \
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
    ModbusBase.cpp \
    ModbusClient.cpp \
    ModbusClientService.cpp \
    ModbusDataConverter.cpp \
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

