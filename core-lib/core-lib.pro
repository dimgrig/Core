QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport #webenginewidgets

TEMPLATE = lib
DEFINES += CORELIB_LIBRARY

CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)


exists(/etc/astra_version) {
    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
}

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

SOURCES += \
    Logger.cpp \
    TestRunner.cpp \
    enum_macro.cpp \
    mixins.cpp \
    Profiler.cpp \
    Sleeper.cpp \
    macroses.cpp \
    templates.cpp \
    templates_cout.cpp

HEADERS += \
    CoreConfig.h \
    Logger.h \
    Random.h \
    enum_macro.h \
    mixins.h \
    Profiler.h \
    Sleeper.h \
    bit.h \
    interfaces.h \
    macroses.h \
    templates.h \
    templates_cout.h \
    visitor.h

BUILD_DIR = $$PWD/../build/$${TARGET}-build/
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
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-lib.so) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-lib.so.1) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-lib.so.1.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-lib.so.1.0.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t))
