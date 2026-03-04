QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += gui widgets sql printsupport

TEMPLATE = lib
DEFINES += CORE_MVC_LIB_LIBRARY

CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

exists(/etc/astra_version) {
    LIBS += -L/usr/lib/qt/ -lcore-lib
    LIBS += -L/usr/lib/qt/ -lcore-utils-lib
    LIBS += -L/usr/lib/qt/ -lcore-widgets-lib

    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L/usr/lib/qt/ -lcore-lib
        LIBS += -L/usr/lib/qt/ -lcore-utils-lib
        LIBS += -L/usr/lib/qt/ -lcore-widgets-lib
    }

    WIN_LIBS_PATH = "C:/QtCreatorProjects/libs"
    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-utils-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-widgets-lib
    }
    win64 {
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-utils-lib
        LIBS += -L$${WIN_LIBS_PATH} -lcore-widgets-lib
    }
}

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-utils-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-widgets-lib"

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
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-mvc-lib.so) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-mvc-lib.so.1) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-mvc-lib.so.1.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-mvc-lib.so.1.0.0) $$quote(/usr/lib/qt/) $$escape_expand(\\n\\t)
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-mvc-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-mvc-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-mvc-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-mvc-lib.a) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-lib.dll) $$quote(C:/QtCreatorProjects/libs) $$escape_expand(\\n\\t))

HEADERS += \
    BaseEditTableModel.h \
    BaseEditTableView.h \
    BaseSqlRelationalTableModel.h \
    BaseTableModel.h \
    BaseTableView.h \
    Completer.h \
    DateDelegate.h \
    DateTimeUTCDelegate.h \
    DoubleSpinBoxDelegate.h \
    EditBar.h \
    FilterHeader.h \
    LineEditDelegate.h \
    LogLevelDelegate.h \
    NumericIdProxyModel.h \
    ReadOnlyDelegate.h \
    SpinBoxDelegate.h \
    StringIdProxyModel.h \
    TableShortcutHandler.h \
    TemplateEditRelationalTableModel.h \
    TemplateEditRelationalTableModel.hpp \
    TemplateEditTableModel.h \
    TemplateEditTableModel.hpp \
    core-mvc-lib-global.h \

SOURCES += \
    BaseEditTableModel.cpp \
    BaseEditTableView.cpp \
    BaseSqlRelationalTableModel.cpp \
    BaseTableModel.cpp \
    BaseTableView.cpp \
    Completer.cpp \
    DateDelegate.cpp \
    DateTimeUTCDelegate.cpp \
    DoubleSpinBoxDelegate.cpp \
    EditBar.cpp \
    FilterHeader.cpp \
    LineEditDelegate.cpp \
    LogLevelDelegate.cpp \
    NumericIdProxyModel.cpp \
    ReadOnlyDelegate.cpp \
    SpinBoxDelegate.cpp \
    StringIdProxyModel.cpp \
    TableShortcutHandler.cpp \
    TemplateEditRelationalTableModel.cpp \
    TemplateEditTableModel.cpp
