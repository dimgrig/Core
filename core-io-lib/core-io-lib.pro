QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network serialport serialbus mqtt

TEMPLATE = lib
DEFINES += CORE_IO_LIB_LIBRARY

CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

exists(/etc/astra_version) {
    LIBS += -L/usr/lib/qt/ -lcore-lib

    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L/usr/lib/qt/ -lcore-lib
    }

    WIN_LIBS_PATH = "C:/QtCreatorProjects/libs"
    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
    }
    win64 {
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
    }
}

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

# include("$$_PRO_FILE_PWD_/QtWebApp/httpserver/httpserver.pri")
# #include("$$_PRO_FILE_PWD_/QtWebApp/logging/logging.pri")
# include("$$_PRO_FILE_PWD_/QtWebApp/templateengine/templateengine.pri")

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/http"
INCLUDEPATH += "$$_PRO_FILE_PWD_/QtWebApp"
INCLUDEPATH += "$$_PRO_FILE_PWD_/QtWebApp/httpserver"
#INCLUDEPATH += "$$_PRO_FILE_PWD_/QtWebApp/logging"
INCLUDEPATH += "$$_PRO_FILE_PWD_/QtWebApp/templateengine"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib"

BUILD_DIR = $$PWD/../build/$${TARGET}-build/
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
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-io-lib.so) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-io-lib.so.1) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-io-lib.so.1.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-io-lib.so.1.0.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-io-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-io-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-io-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-io-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t))

HEADERS += \
    HTTPClient.h \
    MQTTClient.h \
    ResourceManager.h \
    SerialClient.h \
    TCPClient.h \
    TCPServer.h \
    UDPClient.h \
    UDPServer.h \
    core-io-lib-global.h

SOURCES += \
    HTTPClient.cpp \
    MQTTClient.cpp \
    ResourceManager.cpp \
    SerialClient.cpp \
    TCPClient.cpp \
    TCPServer.cpp \
    UDPClient.cpp \
    UDPServer.cpp
