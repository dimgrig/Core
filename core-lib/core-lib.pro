QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += gui printsupport

TEMPLATE = lib
DEFINES += CORELIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

SOURCES += \
    CoreConfig.cpp \
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
    TestRunner.h \
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
