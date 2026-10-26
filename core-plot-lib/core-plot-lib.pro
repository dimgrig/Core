QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TEMPLATE = lib
DEFINES += CORE_PLOT_LIB_LIBRARY
DEFINES += QCUSTOMPLOT_COMPILE_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

HEADERS += \
    CursorManager.h \
    CustomAxisTag.h \
    CustomBarPlot.h \
    CustomPlot.h \
    CustomPlotBase.h \
    CustomPlotCalibration.h \
    CustomPlotDynamic.h \
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
    CustomPlotDynamic.cpp \
    CustomPlotSettings.cpp \
    StatsManager.cpp


CORE_LINK_LIBS = core-lib core-journal-lib qcustomplot
include(../core-build.pri)
VERSION = $$CORE_VERSION

# Install layout — паритет с CMake install (см. cmake/CoreLib.cmake).
unix:!macx {
    target.path   = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_LIBDIR
    headers.files = $$HEADERS
    headers.path  = $$CORE_INSTALL_PREFIX/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
}
win32|win64 {
    # На mingw .dll и .a попадают в lib — qmake INSTALLS не разделяет RUNTIME/ARCHIVE.
    # Для разделения (bin/ для .dll) используется CMake-путь.
    target.path   = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_LIBDIR
    headers.files = $$HEADERS
    headers.path  = $$CORE_APP_LOCAL_DIR/$$CORE_INSTALL_INCLUDEDIR/$$TARGET
}
headers.files =
for(h, HEADERS) {
    headers.files += $$_PRO_FILE_PWD_/$$h
}
INSTALLS += target headers

