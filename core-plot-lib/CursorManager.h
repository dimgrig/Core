#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

#include <QPointer>

#include "qcustomplot.h"

#include "core-plot-lib-global.h"

class CustomPlotBase;

struct CORE_PLOT_LIB_EXPORT ClosestPointInfo {
    QCPGraph *graph = nullptr;
    double key = 0.0;
    double value = 0.0;
    double distance = std::numeric_limits<double>::max();
    bool isValid() const { return graph != nullptr; }
};

class CORE_PLOT_LIB_EXPORT CursorManager {
public:
    enum class Mode {
        NoCursor,
        VerticalCursor,
        HorizontalCursor,
        CrossCursor
    };

    explicit CursorManager(CustomPlotBase *plot);
    ~CursorManager();

    void setMode(Mode mode);
    Mode mode() const { return _mode; }

    void handleMouseMove(const QPoint &pixelPos);
    void handleAxisRangeChanged();

private:
    void createItems();
    void destroyItems();
    QString formatKey(double key, QCPAxis *axis) const;
    void updateVisibility();
    void updatePosition(const QPointF &plotPos, const ClosestPointInfo &pointInfo);
    ClosestPointInfo findClosestPoint(const QPoint &pixelPos, Mode mode) const;

    CustomPlotBase *_plot;
    Mode _mode = Mode::NoCursor;

    QPointer<QCPItemLine> _verticalLine;
    QPointer<QCPItemLine> _horizontalLine;
    QPointer<QCPItemText> _cursorText;
};

#endif // CURSORMANAGER_H
