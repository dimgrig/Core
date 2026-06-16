#include "CustomPlotCalibration.h"

#include "CustomPlotSettings.h"
#include "Logger.h"
#include "Profiler.h"

CustomPlotCalibration::CustomPlotCalibration(QWidget *parent) :
    QCustomPlot(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->setSizePolicy(sizePolicy);
    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, false);
    //this->axisRect()->setAutoMargins(QCP::msNone);
    //this->axisRect()->setMinimumMargins({75, 75, 75, 75});


    form_charts();

    connect(this, &CustomPlotCalibration::customContextMenuRequested, this, &CustomPlotCalibration::context_menu_request);
    connect(this, &CustomPlotCalibration::selectionChangedByUser, this, &CustomPlotCalibration::selection_changed);

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(this, &CustomPlotCalibration::mousePress, this, &CustomPlotCalibration::mouse_press);
    connect(this, &CustomPlotCalibration::mouseWheel, this, &CustomPlotCalibration::mouse_wheel);
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
    this->yAxis->setLabel("Измереенные значения");

    points_graph->selectionDecorator()->setPen(QPen((QColor)DR_COLOR, 1));
    calibration_graph->selectionDecorator()->setPen(QPen((QColor)DG_COLOR, 1));
}

void CustomPlotCalibration::move_legend(int pos)
{
    this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)pos);
    this->replot();
}

void CustomPlotCalibration::mouse_press()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        this->axisRect()->setRangeDrag(this->xAxis->orientation());
    else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        this->axisRect()->setRangeDrag(this->yAxis->orientation());
    else
        this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void CustomPlotCalibration::mouse_wheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        this->axisRect()->setRangeZoom(this->xAxis->orientation());
    else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        this->axisRect()->setRangeZoom(this->yAxis->orientation());
    else
        this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void CustomPlotCalibration::selection_changed() {
    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        this->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        this->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        this->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        this->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i < this->graphCount(); ++i)
    {
        QCPGraph *graph = this->graph(i);
        QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected()) {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}

void CustomPlotCalibration::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
    case Qt::MiddleButton:
        this->yAxis->rescale();
        this->xAxis->rescale();
        this->replot();
        return;
        break;
    default:

        break;
    }

    QCustomPlot::mousePressEvent(event);
}

void CustomPlotCalibration::add_points_graph()
{
    points_graph = this->addGraph(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
    points_graph->setName(graph_names.at(0));
    points_graph->setPen(QPen((QColor)R_COLOR, this->width));
    points_graph->setLineStyle(QCPGraph::lsNone);
    points_graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 10));
    graph_en[graph_names.at(0)] = true;
}

void CustomPlotCalibration::add_calibration_graph()
{
    calibration_graph = this->addGraph(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 2));
    calibration_graph->setName(graph_names.at(1));
    calibration_graph->setPen(QPen((QColor)G_COLOR, this->width));
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

void CustomPlotCalibration::context_menu_request(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (this->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("Переместить в верхний левый угол", [this](){this->move_legend((int)(Qt::AlignTop|Qt::AlignLeft));});
        menu->addAction("Переместить вверху по центру", [this](){this->move_legend((int)(Qt::AlignTop|Qt::AlignHCenter));});
        menu->addAction("Переместить в верхний правый угол", [this](){this->move_legend((int)(Qt::AlignTop|Qt::AlignRight));});
        menu->addAction("Переместить в нижний правый угол", [this](){this->move_legend((int)(Qt::AlignBottom|Qt::AlignRight));});
        menu->addAction("Переместить в нижний левый угол", [this](){this->move_legend((int)(Qt::AlignBottom|Qt::AlignLeft));});
    } else { // general context menu on graphs requested

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
    menu->popup(this->mapToGlobal(pos));
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
