#ifndef CUSTOMBARPLOT_H
#define CUSTOMBARPLOT_H

#include "CustomPlotBase.h"

#include "core-plot-lib-global.h"

class CORE_PLOT_LIB_EXPORT CustomBarPlot : public CustomPlotBase {
Q_OBJECT
public:
    explicit CustomBarPlot(QWidget *parent = nullptr, CustomPlotOrientation orientation = OrientationHorizontal, size_t bar_number = 1, bool text_x_ticker = true);
    ~CustomBarPlot() override;

    void export_data(QTextStream &stream) override;
    void sync_selection() override;
public slots:
    void set_labels(QString yAxisLabel, QString xAxisLabel);
    void set_cp_name(QString name, size_t index = 0);
    void set_x_ticks(QVector<double> ticks, QVector<QString> labels);
    void set_y_tick_step(double max_y_value);
    //void set_cp_data(const CustomPlotData& data);
    //void set_cp_data(std::vector<double> &data, std::vector<double> &ticks);
    void set_cp_data(QVector<double> &keys, QVector<double> &values, size_t index = 0, bool add = false);
    ////void get_cp_data(QVector<int> &tests_v, QVector<double> &tests_failed_v, size_t index = 0);

    void clear();
    void clear(size_t index);
    void clear_all();
private slots:
    void remove_selected_bar();
    void remove_bar(size_t index = 0);
    void remove_all_bars();

    void show_all_bars();
    void show_bar(size_t index = 0);
    void show_bar(const QString name);

    void clear_bar(size_t index = 0);
    void clear_all_bars();

    void context_menu_request(QPoint pos);
private:
    void form_charts();
    void add_bar(size_t index = 0);

    QStringList bar_names;
    std::map<QString, bool> bar_en;
    std::map<QString, std::function<void(int)>> bar_creator;

    std::vector<QPointer<QCPBars>> bar_v;

    CustomPlotOrientation _orientation = OrientationHorizontal;
    size_t _bar_number = 1;

    bool _text_x_ticker = false;
};
#endif // CUSTOMBARPLOT_H
