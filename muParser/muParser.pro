CONFIG -= qt
TEMPLATE = lib
CONFIG += c++11
CONFIG += staticlib
CONFIG += unicode
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

# Заголовки перечислены явно для IDE, но install берёт $$files(include/*.h).
HEADERS += $$files($$_PRO_FILE_PWD_/include/*.h)

# Install layout — паритет с CMake install (см. cmake/CoreLib.cmake).
unix:!macx {
    target.path   = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    headers.files = $$files($$_PRO_FILE_PWD_/include/*.h)
    headers.path  = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/muParser
}
win32|win64 {
    target.path   = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
    headers.files = $$files($$_PRO_FILE_PWD_/include/*.h)
    headers.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/muParser
}
INSTALLS += target headers
