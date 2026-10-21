QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport network

TEMPLATE = lib
DEFINES += CORE_UTILS_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

# --- общая инфраструктура core ---
CORE_LINK_LIBS = core-lib core-journal-lib core-widgets-lib muParser
include(../core-build.pri)
VERSION = $$CORE_VERSION

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
