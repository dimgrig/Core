QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport network #webenginewidgets

TEMPLATE = lib
DEFINES += CORE_UTILS_LIB_LIBRARY

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
    LIBS += -L/usr/local/lib/ -lmuParser

    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-journal-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lmuParser
    }

    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-journal-lib
        LIBS += -L$${WIN_LIBS_PATH} -lmuParser
    }
    win64 {
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-journal-lib
        LIBS += -L$${WIN_LIBS_PATH} -lmuParser
    }
}

DEFINES += MUPARSER_STATIC

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-journal-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../muParser/include"

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

HEADERS += \
    AsyncWaiter.h \
    CQueue.h \
    Calibration.h \
    DelayUtils.h \
    DocxExporter.h \
    FilterMixin.h \
    MathParser.h \
    SessionCache.h \
    dimensions.h \
    functions.h \
    core-utils-lib-global.h \
    functions_degrees.h \
    functions_http.h

SOURCES += \
    AsyncWaiter.cpp \
    CQueue.cpp \
    Calibration.cpp \
    DelayUtils.cpp \
    DocxExporter.cpp \
    FilterMixin.cpp \
    MathParser.cpp \
    SessionCache.cpp \
    functions.cpp \
    functions_degrees.cpp \
    functions_http.cpp

unix {
    wasm {
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.a)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    } else {
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so.1)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so.1.0)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so.1.0.0)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    }
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-utils-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-utils-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t))
