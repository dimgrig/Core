QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TEMPLATE = lib
DEFINES += CORE_JOURNAL_LIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

CORE_LINK_LIBS = core-lib
include(../core-build.pri)
VERSION = $$CORE_VERSION

HEADERS += \
    Categories.h \
    JournalAccess.h \
    JournalBase.h \
    core-journal-lib-global.h

SOURCES += \
    JournalAccess.cpp \
    JournalBase.cpp

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
