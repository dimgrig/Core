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

unix { target.path = /usr/lib/ }
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
