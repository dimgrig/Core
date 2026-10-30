#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include <set>
#include <vector>

#include "CustomPlotBase.h"
#include "CustomPlotSettings.h"

#include "CustomAxisTag.h"

#include "core-plot-lib-global.h"

#ifndef QCUSTOMPLOT_USE_OPENGL
    #define QCUSTOMPLOT_USE_OPENGL
#endif

class CORE_PLOT_LIB_EXPORT CustomPlot : public CustomPlotBase {
Q_OBJECT
public:
    explicit CustomPlot(const QString &objectName, QWidget *parent = nullptr, CustomPlotOrientation orientation = OrientationHorizontal, size_t graph_number = 1, size_t _axis_tag_number = 2);
    ~CustomPlot() override;
public slots:
    void set_labels(const QString& yAxisLabel, const QString& xAxisLabel, size_t index = 0);
    void set_cp_name(const QString& name, size_t index = 0);
    void set_cp_data(const CustomPlotData& data, size_t index = 0);
    void set_cp_data(std::vector<double> &data, std::vector<double> &ticks, size_t index = 0);
    void set_cp_data(QVector<double> &data, QVector<double> &ticks, size_t index = 0, bool add = false);
    void set_axis_tag(float position, size_t index = 0, size_t axis_tag_index = 0);
    void set_axis_tag_v(std::vector<double> axis_tag_v, size_t index = 0);

    //void data_size_changed(int size);
    void clear();
    void clear(size_t index);
    void clear_all();

    void set_range(double x_upper, double x_lower, double y_upper, double y_lower, size_t index = 0);

    void remove_selected_graph();
    void remove_graph(size_t index = 0);
    void remove_all_graphs();
    void remove_axis_tag(size_t index = 0);

    void show_all_graphs();
    void show_graph(size_t index);
    void show_graph(const QString name);
    void show_axis_tag(size_t index);

    void clear_axis_tag(size_t index = 0, bool delete_position = 1);
    void clear_graph(size_t index = 0);
    void clear_all_axis_tag();
    void clear_all_graphs();
private slots:

private:
    CustomPlot(const CustomPlot&) = delete;
    CustomPlot& operator=(const CustomPlot&) = delete;

    virtual void addGraphSpecificActions(QMenu *menu) override;

    void form_charts();
    void add_graph(size_t index,
                   const QString& styleKey = QString(),
                   int colorIndex = -1);

    // std::vector<QString> graph_names;
    // std::vector<std::pair<QString, bool>> graph_en;
    //std::map<QString, std::function<void(int)>> graph_creator;
    std::vector<bool> graph_visible;

    std::vector<QPointer<QCPGraph>> graph_v;
    std::vector<std::vector<double>> _position_v;
    std::vector<std::vector<CustomAxisTag*>> _cat_v;
    std::vector<std::vector<QCPItemStraightLine*>> _infLine_v;

    CustomPlotOrientation _orientation = OrientationHorizontal;
    unsigned int _graph_number = 0;
    unsigned int _axis_tag_number = 0;
};

#endif // CUSTOMPLOT_H
