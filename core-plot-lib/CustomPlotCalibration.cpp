#include "CustomPlotCalibration.h"

#include "CustomPlotSettings.h"
#include "Logger.h"
#include "Profiler.h"

CustomPlotCalibration::CustomPlotCalibration(const QString& objectName, QWidget *parent) :
    CustomPlotBase(objectName, parent)
{
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->setSizePolicy(sizePolicy);
    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, false);
    //this->axisRect()->setAutoMargins(QCP::msNone);
    //this->axisRect()->setMinimumMargins({75, 75, 75, 75});

    form_charts();

    connect(this, &CustomPlotBase::customContextMenuRequested, this, &CustomPlotCalibration::contextMenuRequested);
}

CustomPlotCalibration::~CustomPlotCalibration() {

}

void CustomPlotCalibration::form_charts() {

    //this->setOpenGl(true);
    QCustomPlot::setOpenGl(true);
    QCustomPlot::setAntialiasedElements(QCP::aeNone);

    graph_names.push_back("Показания" + QString("     "));
    graph_names.push_back("Калибровочная хар-ка" + QString("     "));

    //graph_creator[graph_names.at(0)] = std::bind(&CustomPlotCalibration::add_points_graph, this);
    //graph_creator[graph_names.at(1)] = std::bind(&CustomPlotCalibration::add_calibration_graph, this);

    // setup policy and connect slot for context menu popup:
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                          QCP::iSelectLegend | QCP::iSelectPlottables);

    this->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    this->legend->setFont(legendFont);
    this->legend->setSelectedFont(legendFont);
    this->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    this->move_legend((int)(Qt::AlignTop|Qt::AlignLeft));

    // configure bottom axis to show date-time
    //QSharedPointer<QCPAxisTickerDateTime> dt_axis(new QCPAxisTickerDateTime);
    //dt_axis->setDateTimeFormat(DT_FORMAT_SHORT);//DT_FORMAT_WRAP
    //this->xAxis->setTicker(dt_axis);

    this->add_points_graph();
    this->add_calibration_graph();

    // give the axes some labels:
    this->xAxis->setLabel("Заданные значения");
    this->yAxis->setLabel("Измеренные значения");

    points_graph->selectionDecorator()->setPen(QPen((QColor)DR_COLOR, 1));
    calibration_graph->selectionDecorator()->setPen(QPen((QColor)DG_COLOR, 1));

    this->set_width(3);

    // Индексы подобраны так, чтобы цвета по умолчанию совпадали с исходными:
    // defaultGraphColor(0) = G_COLOR, defaultGraphColor(1) = R_COLOR
    points_graph->setProperty("styleKey", "calibration_points");
    points_graph->setProperty("colorIndex", 1);         // R_COLOR
    applySavedStyle(points_graph, "calibration_points");

    calibration_graph->setProperty("styleKey", "calibration_line");
    calibration_graph->setProperty("colorIndex", 0);    // G_COLOR
    applySavedStyle(calibration_graph, "calibration_line");

    // ВАЖНО: applyGraphStyle перезаписывает selectionDecorator на color.darker().
    // Возвращаем оригинальные фиксированные цвета выделения:
    points_graph->selectionDecorator()->setPen(QPen((QColor)DR_COLOR, 1));
    calibration_graph->selectionDecorator()->setPen(QPen((QColor)DG_COLOR, 1));
}

void CustomPlotCalibration::add_points_graph()
{
    points_graph = this->addGraph(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
    points_graph->setName(graph_names.at(0));
    points_graph->setPen(QPen((QColor)R_COLOR, this->get_width()));
    points_graph->setLineStyle(QCPGraph::lsNone);
    points_graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 10));
    graph_en[graph_names.at(0)] = true;
}

void CustomPlotCalibration::add_calibration_graph()
{
    calibration_graph = this->addGraph(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 2));
    calibration_graph->setName(graph_names.at(1));
    calibration_graph->setPen(QPen((QColor)G_COLOR, this->get_width()));
    calibration_graph->setLineStyle(QCPGraph::lsLine); //lsStepRight
    graph_en[graph_names.at(1)] = true;
}

void CustomPlotCalibration::remove_selected_graph()
{
    if (!this->selectedGraphs().empty())
    {
        this->legend->selectedItems().first()->setVisible(false);
        graph_en[this->selectedGraphs().first()->name()] = false;
        this->selectedGraphs().first()->setVisible(false);
        //cp->removeGraph(cp->selectedGraphs().first());
        this->replot();
    }
}

void CustomPlotCalibration::remove_all_graphs()
{
    for (int i = 0; i < this->graphCount(); i++) {
        this->graph(i)->setVisible(false);
        graph_en[this->graph(i)->name()] = false;
        this->legend->item(i)->setVisible(false);
    }
    this->replot();
}

void CustomPlotCalibration::set_axis_labels(const QString xAxisLabel, const QString yAxisLabel) {
    this->xAxis->setLabel(xAxisLabel);
    this->yAxis->setLabel(yAxisLabel);
}

void CustomPlotCalibration::set_cp_data(const QCPDataContainer<QCPGraphData> &points_data, const QCPDataContainer<QCPGraphData> &calibration_data) {
    {
        points_graph->data()->clear();
        calibration_graph->data()->clear();

        points_graph->data()->add(points_data);
        calibration_graph->data()->add(calibration_data);
    }
    if (this->visibleRegion() != QRegion()) {
        this->rescale();
    }
}

void CustomPlotCalibration::rescale() {
    {
        PROFILER("rescale");

        this->rescaleAxes();
    }
    {
        PROFILER("replot");
        this->replot(QCustomPlot::rpQueuedReplot);
    }
}

void CustomPlotCalibration::addGraphSpecificActions(QMenu *menu)
{
    // Показать скрытые графики
    for (auto [name, en] : graph_en) {
        if (!en) {
            menu->addAction("Показать график \"" + name + "\"", [this, name](){
                for (int i = 0; i < this->graphCount(); i++) {
                    if (name == this->graph(i)->name()) {
                        this->graph(i)->setVisible(true);
                        graph_en[this->graph(i)->name()] = true;
                        this->legend->item(i)->setVisible(true);
                        this->rescale();
                    }
                }
            });
        }
    }
    if (this->selectedGraphs().size() > 0)
        menu->addAction("Скрыть выбранный график", this, &CustomPlotCalibration::remove_selected_graph);
    if (this->graphCount() > 0)
        menu->addAction("Скрыть все графики", this, &CustomPlotCalibration::remove_all_graphs);
}
