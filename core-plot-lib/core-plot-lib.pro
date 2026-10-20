QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TEMPLATE = lib
DEFINES += CORE_PLOT_LIB_LIBRARY
DEFINES += QCUSTOMPLOT_COMPILE_LIBRARY
CONFIG += c++17

INCLUDEPATH += "$$_PRO_FILE_PWD_/"

CORE_LINK_LIBS = core-lib core-journal-lib qcustomplot
include(../core-build.pri)

unix { target.path = /usr/lib/ }
!isEmpty(target.path): INSTALLS += target

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
