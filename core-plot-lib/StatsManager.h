#ifndef STATSMANAGER_H
#define STATSMANAGER_H

#include <QPointer>
#include <QVector>
#include "qcustomplot.h"

#include "core-plot-lib-global.h"

class CustomPlotBase;

class CORE_PLOT_LIB_EXPORT StatsManager {
public:
    struct GraphStats {
        QString name;
        double minValue = 0.0;
        double maxValue = 0.0;
        double mean = 0.0;
        double stdDev = 0.0;
        int pointCount = 0;
    };

    explicit StatsManager(CustomPlotBase *plot);
    ~StatsManager();

    void setVisible(bool visible);
    bool isVisible() const { return _visible; }

    void updateStats();  // пересчитать статистику по всем видимым графикам
    void setShowOnlyVisibleRange(bool onlyVisible);
    bool showOnlyVisibleRange() const { return _onlyVisibleRange; }

private:
    void createTextItem();
    void destroyTextItem();
    void updateTextDisplay();
    GraphStats computeStatsForGraph(QCPGraph *graph) const;

    CustomPlotBase *_plot;
    bool _visible = false;
    bool _onlyVisibleRange = true; // считать только по отображаемой области осей

    QPointer<QCPItemText> _statsText;
};

#endif // STATSMANAGER_H
