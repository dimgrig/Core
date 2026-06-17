QT       += core
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport #webenginewidgets

TEMPLATE = lib
DEFINES += CORE_PLOT_LIB_LIBRARY
DEFINES += QCUSTOMPLOT_COMPILE_LIBRARY

CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-core-version.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-core-version.pri)
include(../qmake-destination-path.pri)

exists(/etc/astra_version) {
    LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
    LIBS += -L$${LINUX_LIBS_PATH} -lqcustomplot

    DISTFILES += ../core-gcc10.pri
    include(../core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L$${LINUX_LIBS_PATH} -lcore-lib
        LIBS += -L$${LINUX_LIBS_PATH} -lqcustomplot
    }

    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lqcustomplot2
    }
    win64 {
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lqcustomplot2
    }
}

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

INCLUDEPATH += "$$_PRO_FILE_PWD_/../core-lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../qcustomplot"

DESTDIR = $$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR = $$PWD/../$../${TARGET}-build/$$DESTINATION_PATH/.ui

# Default rules for deployment.
unix {
    target.path = /usr/lib/
}
!isEmpty(target.path): INSTALLS += target

unix {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-plot-lib.so)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-plot-lib.so.1)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-plot-lib.so.1.0)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-plot-lib.so.1.0.0)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-plot-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-plot-lib.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/core-plot-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libcore-plot-lib.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}

#message($$QMAKE_COPY $$quote($$shell_path($$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH/core-plot-lib.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t))

HEADERS += \
    CursorManager.h \
    CustomAxisTag.h \
    CustomBarPlot.h \
    CustomPlot.h \
    CustomPlotBase.h \
    CustomPlotCalibration.h \
    CustomPlotSettings.h \
    StatsManager.h \
    core-plot-lib-global.h \

SOURCES += \
    CursorManager.cpp \
    CustomAxisTag.cpp \
    CustomBarPlot.cpp \
    CustomPlot.cpp \
    CustomPlotBase.cpp \
    CustomPlotCalibration.cpp \
    CustomPlotSettings.cpp \
    StatsManager.cpp
