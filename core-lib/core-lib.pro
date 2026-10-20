QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TEMPLATE = lib
DEFINES += CORELIB_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

include(../core-build.pri)

unix { target.path = /usr/lib/ }
!isEmpty(target.path): INSTALLS += target

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
