core_add_library(core-plot-lib
    QT Core PrintSupport
    DEPENDS core-lib core-journal-lib qcustomplot
    #DEFINES QCUSTOMPLOT_COMPILE_LIBRARY
)

target_link_libraries(core-plot-lib PRIVATE qcustomplot)

target_sources(core-plot-lib PRIVATE
    CursorManager.cpp
    CustomAxisTag.cpp
    CustomBarPlot.cpp
    CustomPlot.cpp
    CustomPlotBase.cpp
    CustomPlotCalibration.cpp
    CustomPlotDynamic.cpp
    CustomPlotSettings.cpp
    StatsManager.cpp

    CursorManager.h
    CustomAxisTag.h
    CustomBarPlot.h
    CustomPlot.h
    CustomPlotBase.h
    CustomPlotCalibration.h
    CustomPlotDynamic.h
    CustomPlotSettings.h
    StatsManager.h
    core-plot-lib-global.h
)

