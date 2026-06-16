#ifndef CUSTOMPLOTBASE_H
#define CUSTOMPLOTBASE_H

#include <QWidget>
#include "qcustomplot.h"

#include "CoreConfig.h"

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

    virtual void export_data(QTextStream &stream) = 0;
    virtual void sync_selection() = 0;

    bool get_autorescale() const {return this->_autorescale;}
    void set_autorescale(bool autorescale) {this->_autorescale = autorescale;}

    void rescale(int index = 0);
    void autoscale_all();
protected slots:
    void move_legend(int pos);
    int get_width() const {return _width;};

    void export_csv();
    void export_png();
private slots:
    void mouse_press();
    void mouse_wheel();
    void selection_changed();
private:
    void mousePressEvent(QMouseEvent *event) override;

    bool _autorescale = true;

    int _width = 1;
};

#endif // CUSTOMPLOTBASE_H
