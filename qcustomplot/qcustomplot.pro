#
#  Project to compile QCustomPlot as shared library (.so/.dll) from the amalgamated sources
#

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

DEFINES += QCUSTOMPLOT_COMPILE_LIBRARY
TEMPLATE = lib
CONFIG += release build_all #debug_and_release
static {
  CONFIG += static
} else {
  CONFIG += shared
}

VERSION = 2.1.1

#TARGET = qcustomplot
CONFIG(debug, debug|release) {
  TARGET = $$join(TARGET,,,d) # if compiling in debug mode, append a "d" to the library name
  QMAKE_TARGET_PRODUCT = "QCustomPlot (debug mode)"
  QMAKE_TARGET_DESCRIPTION = "Plotting library for Qt (debug mode)"
} else {
  QMAKE_TARGET_PRODUCT = "QCustomPlot"
  QMAKE_TARGET_DESCRIPTION = "Plotting library for Qt"
}
QMAKE_TARGET_COMPANY = "www.qcustomplot.com"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) by Emanuel Eichhammer"



SOURCES += qcustomplot.cpp
HEADERS += qcustomplot.h



DISTFILES += \
    ../qmake-destination-path.pri \
    ../qmake-core-version.pri \
    ../qmake-target-platform.pri

include(../qmake-target-platform.pri)
include(../qmake-core-version.pri)
include(../qmake-destination-path.pri)

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

DESTDIR = $$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR = $$PWD/../../$${TARGET}-build/$$DESTINATION_PATH/.ui

unix {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libqcustomplot.so)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libqcustomplot.so.2)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libqcustomplot.so.2.1)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libqcustomplot.so.2.1.1)) $$quote($$shell_path($${LINUX_LIBS_PATH})) $$escape_expand(\\n\\t)
}

win32 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/qcustomplot2.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libqcustomplot2.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}
win64 {
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/qcustomplot2.dll)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/../../$${TARGET}-binaries/$$DESTINATION_PATH/libqcustomplot2.a)) $$quote($$shell_path($${WIN_LIBS_PATH})) $$escape_expand(\\n\\t)
}
