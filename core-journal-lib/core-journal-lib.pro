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

unix { target.path = /usr/lib/ }
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Categories.h \
    JournalAccess.h \
    JournalBase.h \
    core-journal-lib-global.h

SOURCES += \
    JournalAccess.cpp \
    JournalBase.cpp
