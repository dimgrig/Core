#ifndef CUSTOMPLOTCALIBRATION_H
#define CUSTOMPLOTCALIBRATION_H

#include <set>
#include <vector>

#include "qcustomplot.h"

#include "core-plot-lib-global.h"

class CORE_PLOT_LIB_EXPORT CustomPlotCalibration : public QCustomPlot {
    Q_OBJECT
public:
    explicit CustomPlotCalibration(QWidget *parent = nullptr);
    ~CustomPlotCalibration() override;
public slots:
    void set_axis_labels(const QString xAxisLabel, const QString yAxisLabel);
    void set_cp_data(const QCPDataContainer<QCPGraphData> &points_data, const QCPDataContainer<QCPGraphData> &calibration_data);
    void rescale();
private slots:
    void move_legend(int pos);

    void mouse_press();
    void mouse_wheel();
    void selection_changed();

    void add_points_graph();
    void add_calibration_graph();

    void remove_selected_graph();
    void remove_all_graphs();
    void context_menu_request(QPoint pos);
private:
    void form_charts();

    void mousePressEvent(QMouseEvent *event) override;

    QStringList graph_names;
    std::map<QString, bool> graph_en;
    //std::map<QString, std::function<void()>> graph_creator;

    QPointer<QCPGraph> points_graph;
    QPointer<QCPGraph> calibration_graph;

    int width = 3;
};

#endif //CUSTOMPLOTCALIBRATION_H
