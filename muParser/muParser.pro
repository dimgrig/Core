CONFIG -= qt
TEMPLATE = lib
CONFIG += c++11
CONFIG += staticlib
TARGET = muParser

DEFINES += QT_DEPRECATED_WARNINGS MUPARSER_STATIC
QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-cast-function-type

INCLUDEPATH += "$$_PRO_FILE_PWD_/include"

CONFIG += core_no_default_defines
include(../core-build.pri)

SOURCES += \
    src/muParser.cpp \
    src/muParserBase.cpp \
    src/muParserBytecode.cpp \
    src/muParserCallback.cpp \
    src/muParserError.cpp \
    src/muParserInt.cpp \
    src/muParserTest.cpp \
    src/muParserTokenReader.cpp

HEADERS +=

