CONFIG -= qt
TEMPLATE = lib
CONFIG += c++11
CONFIG += staticlib
CONFIG += unicode
TARGET = muParser

DEFINES += QT_DEPRECATED_WARNINGS MUPARSER_STATIC
QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-cast-function-type

INCLUDEPATH += "$$_PRO_FILE_PWD_/include"

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

CONFIG += core_no_default_defines
include(../core-build.pri)

# Install layout — паритет с CMake install (см. cmake/CoreLib.cmake).
unix:!macx {
    target.path   = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    headers.files = $$files($$_PRO_FILE_PWD_/include/*.h)
    headers.path  = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/muParser
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
# НЕ пересобираем headers.files через _PRO_FILE_PWD_/$$h
# HEADERS содержат абсолютные пути ($$files($$_PRO_FILE_PWD_/include/*.h)),
# и префикс даёт двойной путь. headers.files уже назначен корректно.
INSTALLS += target headers
