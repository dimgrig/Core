#ifndef CUSTOMPLOTBASE_H
#define CUSTOMPLOTBASE_H

#include <QWidget>

#include "qcustomplot.h"

#include "CoreConfig.h"

#include "CursorManager.h"
#include "StatsManager.h"

#include "core-plot-lib-global.h"

static std::vector<QString> PEN_COLOR_v {G_COLOR, R_COLOR, B_COLOR};
static std::vector<QString> SELECTION_PEN_COLOR_v {DG_COLOR, DR_COLOR, DB_COLOR};
static std::vector<QString> AXIS_TAG_PEN_COLOR_v {LG_COLOR, _LR_COLOR, LB_COLOR};

class CORE_PLOT_LIB_EXPORT CustomPlotBase : public QCustomPlot {
Q_OBJECT
public:
    explicit CustomPlotBase(QWidget *parent = nullptr);
    ~CustomPlotBase() override;

    enum CustomPlotOrientation {
        OrientationHorizontal = 0,
        OrientationVertical = 1
    };

    virtual void export_data(QTextStream &stream);
    virtual void sync_selection();

    bool get_autorescale() const {return this->_autorescale;}
    void set_autorescale(bool autorescale) {this->_autorescale = autorescale;}

    bool get_legend_state() const {return this->_legend_state;}
    void set_legend_state(bool state);

    int get_width() const {return _width;}

    void setCursorMode(CursorManager::Mode mode);
    CursorManager::Mode cursorMode() const;

    void setStatsVisible(bool visible);
    bool statsVisible() const;
    void setStatsOnlyVisibleRange(bool onlyVisible);
    bool statsOnlyVisibleRange() const;
    void updateStats(); // принудительное обновление
    void updateStatsIfVisible();

    void syncLegendVisibility();

    void rescale(int index = 0);
    void autoscale_all();
protected slots:
    void move_legend(int pos);

    void export_csv();
    void export_png();

    virtual void mouse_press();
    virtual void mouse_wheel();
    virtual void selection_changed();

    void contextMenuRequested(QPoint pos);
private slots:

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

    std::unique_ptr<CursorManager> _cursorManager;
    std::unique_ptr<StatsManager> _statsManager;

    QList<QCPAxis*> _rightAxes;   // все правые оси (включая активную)
private:
    virtual void addGraphSpecificActions(QMenu *menu) {Q_UNUSED(menu)}

    void mousePressEvent(QMouseEvent *event) override;

    bool _autorescale = true;
    bool _legend_state = true;

    int _width = 1;
};

#endif // CUSTOMPLOTBASE_H
