QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport #webenginewidgets

TEMPLATE = lib
DEFINES += CORE_UTILS_LIB_LIBRARY

CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

exists(/etc/astra_version) {
    LIBS += -L/usr/lib/qt/ -lcore-lib
    LIBS += -L/usr/lib/qt/ -lcore-widgets-lib

    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L/usr/lib/qt/ -lcore-lib
        LIBS += -L/usr/lib/qt/ -lcore-widgets-lib
    }

    WIN_LIBS_PATH = "C:/QtCreatorProjects/libs"
    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-widgets-lib
    }
    win64 {
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-widgets-lib
    }
}

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/muParser"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-widgets-lib"

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
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so.1) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so.1.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.so.1.0.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-utils-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-utils-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-utils-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t))

HEADERS += \
    Approximation.h \
    AsyncWaiter.h \
    CQueue.h \
    DocxExporter.h \
    FilterMixin.h \
    MathParser.h \
    SessionCache.h \
    functions.h \
    muParser/muParser.h \
    muParser/muParserBase.h \
    muParser/muParserBytecode.h \
    muParser/muParserCallback.h \
    muParser/muParserComplex.h \
    muParser/muParserDLL.h \
    muParser/muParserDef.h \
    muParser/muParserError.h \
    muParser/muParserFixes.h \
    muParser/muParserInt.h \
    muParser/muParserStack.h \
    muParser/muParserTest.h \
    muParser/muParserToken.h \
    muParser/muParserTokenReader.h \
    core-utils-lib-global.h

SOURCES += \
    Approximation.cpp \
    AsyncWaiter.cpp \
    CQueue.cpp \
    DocxExporter.cpp \
    FilterMixin.cpp \
    MathParser.cpp \
    SessionCache.cpp \
    functions.cpp \
    muParser/muParser.cpp \
    muParser/muParserBase.cpp \
    muParser/muParserBytecode.cpp \
    muParser/muParserCallback.cpp \
    muParser/muParserComplex.cpp \
    muParser/muParserDLL.cpp \
    muParser/muParserError.cpp \
    muParser/muParserInt.cpp \
    muParser/muParserTest.cpp \
    muParser/muParserTokenReader.cpp

