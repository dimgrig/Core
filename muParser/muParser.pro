CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS MUPARSER_STATIC

QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-cast-function-type

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

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

INCLUDEPATH += include

BUILD_DIR = $$PWD/../../build/$${TARGET}-build/
DESTDIR = $$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.ui

# Default rules for deployment.
unix {
    target.path = /usr/lib/
}
!isEmpty(target.path): INSTALLS += target

unix {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libmuParser.a)) $$quote($$shell_path(/usr/lib/qt/)) $$escape_expand(\\n\\t)
}

win32 {
    #QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/muParser.dll)) $$quote($$shell_path(C:/QtCreatorProjects/libs)) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libmuParser.a)) $$quote($$shell_path(C:/QtCreatorProjects/libs)) $$escape_expand(\\n\\t)
}
win64 {
    #QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/muParser.dll)) $$quote($$shell_path(C:/QtCreatorProjects/libs)) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libmuParser.a)) $$quote($$shell_path(C:/QtCreatorProjects/libs)) $$escape_expand(\\n\\t)
}
