QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TEMPLATE = lib
DEFINES += CORELIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

include(../core-build.pri)
VERSION = $$CORE_VERSION

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
