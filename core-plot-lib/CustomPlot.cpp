#include "CustomPlot.h"

#define HIDE_POSITION       -10000000
#define EXPORT_KEY_TIME 1

#include "CoreConfig.h"
#include "Logger.h"
#include "Profiler.h"
#include "templates.h"

CustomPlot::CustomPlot(QWidget *parent, CustomPlotOrientation orientation, size_t graph_number, size_t axis_tag_number) :
        CustomPlotBase(parent),
        _orientation(orientation),
        _graph_number(graph_number),
        _axis_tag_number(axis_tag_number)
{
    form_charts();

    graph_visible.resize(_graph_number, true);

    for(size_t i = 0; i < _graph_number; ++i) {
        std::vector<double> position_v;
        std::vector<CustomAxisTag*> cat_v;
        std::vector<QCPItemStraightLine*> infLine_v;
        for (size_t j = 0; j < _axis_tag_number; ++j) {
            CustomAxisTag *cat;
            if ((i % 2) == 0) {
                cat = new CustomAxisTag(this->yAxis);
            } else {
                cat = new CustomAxisTag(this->axisRect()->axis(QCPAxis::atRight, 1));
            }
            cat->setPen(this->graph_v[i]->pen());
            double position = HIDE_POSITION;
            cat->updatePosition(position);
            auto infLine = new QCPItemStraightLine(this);
            infLine->setPen(QPen((QColor)AXIS_TAG_PEN_COLOR_v[i], 2));
            infLine->point1->setCoords(position, position);  // location of point 1 in plot coordinate
            infLine->point2->setCoords(position, position);  // location of point 2 in plot coordinate
            if ((i % 2) == 0) {
                infLine->setClipAxisRect(this->axisRect());
                infLine->point1->setAxes(xAxis, yAxis);
                infLine->point2->setAxes(xAxis, yAxis);
            } else {
                infLine->setClipAxisRect(this->axisRect());
                infLine->point1->setAxes(xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
                infLine->point2->setAxes(xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
            }
            infLine->setClipToAxisRect(true);
            cat_v.push_back(cat);
            infLine_v.push_back(infLine);
            position_v.push_back(position);
        }
        this->_position_v.push_back(position_v);
        this->_cat_v.push_back(cat_v);
        this->_infLine_v.push_back(infLine_v);
    }

//    qCDebug(logDebug) << this->plottingHints();
    this->setPlottingHint(QCP::PlottingHint::phFastPolylines, true);
    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, true);

    connect(this, &CustomPlotBase::customContextMenuRequested, this, &CustomPlot::contextMenuRequested);
}

CustomPlot::~CustomPlot() {

}

void CustomPlot::addGraphSpecificActions(QMenu *menu) {
    for (size_t i = 0; i < graph_v.size(); ++i) {
        if (!graph_visible[i]) {
            menu->addAction(tr("Показать график \"%1\"").arg(graph_v[i]->name()),
                            [this, i]() { show_graph(i); });
        }
    }
    if (selectedGraphs().size() > 0)
        menu->addAction("Скрыть выбранный график", this, &CustomPlot::remove_selected_graph);
    if (graphCount() > 0)
        menu->addAction("Скрыть все графики", this, &CustomPlot::remove_all_graphs);
    if (get_legend_state()) {
        menu->addAction("Скрыть легенду", [this](){ set_legend_state(false); });
    } else {
        menu->addAction("Показать легенду", [this](){ set_legend_state(true); });
    }
    menu->addAction("Очистить график", this, &CustomPlot::clear_all_graphs);
}

void CustomPlot::form_charts() {
    //this->setOpenGl(true);
    QCustomPlot::setOpenGl(true);
    QCustomPlot::setAntialiasedElements(QCP::aeNone);

    // for (size_t i = 0; i < _graph_number; ++i) {
    //     if (this->_orientation == OrientationHorizontal) {
    //         graph_names.push_back("Осциллограмма");
    //     }
    //     if (this->_orientation == OrientationVertical) {
    //         graph_names.push_back("Гистограмма");
    //     }

    //     //graph_creator[graph_names.at(i)] = std::bind(&CustomPlot::add_graph, this, i);
    // }

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
    this->move_legend((int)(Qt::AlignBottom|Qt::AlignRight)); //Qt::AlignTop|Qt::AlignLeft

    //top axes
    this->axisRect()->setupFullAxesBox();

    // configure bottom axis to show date-time
    QSharedPointer<QCPAxisTickerDateTime> dt_axis(new QCPAxisTickerDateTime);
    dt_axis->setDateTimeFormat(DATE_TIME_FORMAT_WITHOUT_MS_WRAP);
    this->xAxis->setTicker(dt_axis);

    //right axe
    this->axisRect()->addAxis(QCPAxis::atRight);
    this->axisRect()->axis(QCPAxis::atRight, 1)->setVisible(false);
    this->axisRect()->axis(QCPAxis::atRight, 1)->setLabel("ADC");
    QCPAxis *rightAxis = axisRect()->axis(QCPAxis::atRight, 1);
    if (rightAxis)
        _rightAxes.append(rightAxis);

//    //right axe for temp with tag
//    this->yAxis2->setVisible(true);
    this->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(100); // add some padding to have space for tags
    this->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(50); // add some padding to have space for tags
//    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2, SLOT(setRange(QCPRange))); // left axis only mirrors inner right axis

    for (size_t i = 0; i < _graph_number; ++i) {
        this->add_graph(i);
    }

    // give the axes some labels:
    this->xAxis->setLabel("Sample");
    this->yAxis->setLabel("ADC");
}

void CustomPlot::add_graph(size_t index)
{
    if (_orientation == OrientationVertical) {
        graph_v.push_back(this->addGraph(this->yAxis, this->xAxis));
    }
    if (_orientation == OrientationHorizontal) {
        if ((index % 2) == 0) {
            graph_v.push_back(this->addGraph(this->xAxis, this->yAxis));
        } else {
            graph_v.push_back(this->addGraph(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1)));
        }
    }

    // Устанавливаем имя по умолчанию (локализованное)
    QString defaultName;
    if (_orientation == OrientationHorizontal)
        defaultName = tr("Осциллограмма %1").arg(index + 1);
    else
        defaultName = tr("Гистограмма %1").arg(index + 1);
    graph_v.back()->setName(defaultName);

    QString PEN_COLOR = G_COLOR;
    QString SELECTION_PEN_COLOR = DG_COLOR;
    if (index < PEN_COLOR_v.size()) {
        PEN_COLOR = PEN_COLOR_v.at(index);
        SELECTION_PEN_COLOR = SELECTION_PEN_COLOR_v.at(index);
    }
    // if (index < graph_v.size()) {
    //     graph_v.at(index)->setName(graph_names.at(index));
    //     graph_v.at(index)->setPen(QPen((QColor)PEN_COLOR, this->get_width()));
    //     graph_v.at(index)->selectionDecorator()->setPen(QPen((QColor)SELECTION_PEN_COLOR, 1));
    //     graph_v.at(index)->setLineStyle(QCPGraph::lsLine);
    //     graph_en.push_back(std::make_pair(graph_names.at(index), true));
    // }
    graph_v.back()->setPen(QPen((QColor)PEN_COLOR, this->get_width()));
    graph_v.back()->selectionDecorator()->setPen(QPen((QColor)SELECTION_PEN_COLOR, 1));
    graph_v.back()->setLineStyle(QCPGraph::lsLine);
}

void CustomPlot::remove_selected_graph()
{
    if (selectedGraphs().isEmpty())
        return;

    QCPGraph *selected = selectedGraphs().first();
    int index = -1;
    for (unsigned int i = 0; i < graph_v.size(); ++i) {
        if (graph_v[i] == selected) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        selected->setVisible(false);
        graph_visible[index] = false;
        if (auto item = legend->itemWithPlottable(selected))
            item->setVisible(false);

        remove_axis_tag(index);
        replot(QCustomPlot::rpQueuedReplot);
    }
}

void CustomPlot::remove_graph(size_t index) {
    if (index >= graph_v.size())
        return;

    graph_v[index]->setVisible(false);
    graph_visible[index] = false;
    if (auto item = legend->itemWithPlottable(graph_v[index]))
        item->setVisible(false);

    this->syncLegendVisibility();
    remove_axis_tag(index);
    replot(QCustomPlot::rpQueuedReplot);

    this->updateStatsIfVisible();
}

void CustomPlot::remove_all_graphs() {
    for (int index = 0; index < this->graphCount(); index++) {
        remove_graph(index);
    }
}

void CustomPlot::remove_axis_tag(size_t index) {
    this->clear_axis_tag(index, 0);
}

void CustomPlot::show_all_graphs() {
    for (int index = 0; index < this->graphCount(); index++) {
        show_graph(index);
    }
}

void CustomPlot::show_graph(size_t index) {
    if (index >= graph_v.size())
        return;

    graph_v[index]->setVisible(true);
    graph_visible[index] = true;
    if (auto item = legend->itemWithPlottable(graph_v[index]))
        item->setVisible(true);

    if (_orientation == OrientationHorizontal && (index % 2) == 1)
        axisRect()->axis(QCPAxis::atRight, 1)->setVisible(true);

    this->syncLegendVisibility();
    show_axis_tag(index);
    rescale(index);

    this->updateStatsIfVisible();
}

void CustomPlot::show_graph(const QString name) {
    for (unsigned int i = 0; i < graph_v.size(); ++i) {
        if (graph_v[i] && graph_v[i]->name() == name) {
            show_graph(i);
            break; // предполагаем уникальность имён
        }
    }
}

void CustomPlot::show_axis_tag(size_t index) {
    if (index < this->graph_v.size()) {
        uint8_t axis_tag_index = 0;
        for (auto &position : _position_v[index]) {
            if (position != HIDE_POSITION) {
                this->set_axis_tag(position, index, axis_tag_index);
            }
            axis_tag_index++;
        }
    }
}

void CustomPlot::clear_axis_tag(size_t index, bool delete_postion) {
    double position = HIDE_POSITION;
    if (index < this->_cat_v.size()) {
        uint8_t axis_tag_index = 0;
        for (auto &cat : _cat_v[index]) {
            if (delete_postion) {
                _position_v[index][axis_tag_index] = position;
            }
            cat->setText("");
            cat->updatePosition(position);
            axis_tag_index++;
        }
    }
    if (index < this->_infLine_v.size()) {
        for (auto &infLine : _infLine_v[index]) {
            infLine->point1->setCoords(position, position);
            infLine->point2->setCoords(position, position);
        }
    }
}

void CustomPlot::clear_graph(size_t index) {
    if (index < this->graph_v.size()) {
        QVector<double> empty;
        graph_v[index]->setData(empty, empty); //graph_v[index]->data()->clear();
        //this->rescale(index); // no need?
        this->clear_axis_tag(index);
        graph_v[index]->rescaleAxes();
        this->replot(QCustomPlot::rpQueuedReplot);
    }

    this->updateStatsIfVisible();
}

void CustomPlot::clear_all_axis_tag() {
    for(size_t index = 0; index < this->_graph_number; ++index) {
        this->clear_axis_tag(index);
    }
}

void CustomPlot::clear_all_graphs() {    
    for(unsigned int index = 0; index < this->_graph_number; ++index) {
        this->clear_graph(index);
    }
}

void CustomPlot::set_cp_name(const QString& name, size_t index) {
    if (index < graph_v.size()) {
        // graph_names[index] = name;
        // graph_v.at(index)->setName(name);
        // graph_en[index].first = name;
        // graph_en[index].second = true;
        graph_v.at(index)->setName(name);
        graph_visible[index] = true;
    }
}

void CustomPlot::set_cp_data(const CustomPlotData& data, size_t index) {
    //Profiler pr("CP set_cp_data");

    if (index < graph_v.size()) {
        graph_v[index]->data()->add(data._v);
    }

    this->show_graph(index);

    if (this->visibleRegion() != QRegion()) {
        this->rescale(index);
    }
}

void CustomPlot::set_labels(const QString& yAxisLabel, const QString& xAxisLabel, size_t index) {
    if ((index % 2) == 0) {
        this->yAxis->setLabel(yAxisLabel);
    } else {
        this->axisRect()->axis(QCPAxis::atRight, 1)->setLabel(yAxisLabel);
    }
    this->xAxis->setLabel(xAxisLabel);
}

void CustomPlot::set_cp_data(std::vector<double> &data, std::vector<double> &ticks, size_t index) {
    //Profiler pr("CP SCPD"); //12 ms
    {
//        QVector<double> keys;
//        QVector<double> values;
//        {
//            Profiler pr("CP SCPD QVector"); // 8 ms
//            for(const auto &[value, key] : zip(data, ticks)) {
//                keys.push_back(key);
//                values.push_back(value);
//            }
//        }
        if (index < graph_v.size()) {
            //Profiler pr1("CP SCPD setData"); // 12 ms QVector and 12 mcs setData
            graph_v[index].data()->setData(QVector(ticks.begin(), ticks.end()), QVector(data.begin(), data.end()), true);
        }
    }
    {
        //Profiler pr2("CP SCPD show_graph"); //26 mcs
        this->show_graph(index);
    }

//    if (this->visibleRegion() != QRegion()) { //rescale after add axis_tag
//        this->rescale(index);
//    }
}

void CustomPlot::set_cp_data(QVector<double> &data, QVector<double> &ticks, size_t index, bool add) {
    //Profiler pr("CP SCPD"); //1-2 ms
    {
        if (index < graph_v.size()) {
            if (add) {
                // double last_tick = (graph_v[index]->data()->end() - 1)->key;
                // //qCDebug(logDebug) << last_tick;
                // for(double&tick : ticks) {
                //     tick += last_tick;
                // }
                graph_v[index].data()->addData(ticks, data, true);
            } else {
                graph_v[index].data()->setData(ticks, data, true);
            }
        }
    }
    this->show_graph(index);
}

void CustomPlot::set_axis_tag(float position, size_t index, size_t axis_tag_index) {
    if (index < this->_cat_v.size()) {
        if ((size_t)axis_tag_index < this->_cat_v[index].size()) {
            _position_v.at(index).at(axis_tag_index) = position;
            _cat_v.at(index).at(axis_tag_index)->updatePosition(position);
            if (this->_orientation == CustomPlotOrientation::OrientationHorizontal) {
                //Value value = Value("", position, su_from_name(this->graph(index)->name().toStdString().c_str()), 2);
                //_cat_v.at(index).at(axis_tag_index)->setText(value.get_string_fit_dimension().c_str());
            } else {
                _cat_v.at(index).at(axis_tag_index)->setText(QLocale().toString(position, 10, 3));
            }

//            if ((position > 1000.0) || (position < 0.001)) {
//                //QLocale locale;
//                //_cat_v.at(index).at(axis_tag_index)->setText(QLocale().toString(position, 'e', 3)); //locale.toString(position, 'f', 2)
//                QStringList sl = QLocale().toString(position, 'e', 3).split("e");

//                QString str;
//                if (sl.size() == 2) {
//                    str += sl.front();
//                    str += QChar(0xB7);
//                    str += "10";
//                    str += "<sup>";
//                    str += sl.back().remove(QRegExp("^[0]*"));;
//                    str += "</sup>";
//                    qCDebug(logDebug) << str;
//                } else {
//                    str = QLocale().toString(position, 'e', 3);
//                }

//                _cat_v.at(index).at(axis_tag_index)->setText(str);
//            } else {
//                //_cat_v.at(index).at(axis_tag_index)->setText(locale.toString(position, 'f', 3));
//                _cat_v.at(index).at(axis_tag_index)->setText(QLocale().toString(position, 10, 3).remove(QRegExp("0+$")).remove(QRegExp("\\,$")).remove(QRegExp("\\s+")));
//            }
        }
        if ((size_t)axis_tag_index < this->_infLine_v[index].size()) {
            _infLine_v.at(index).at(axis_tag_index)->point1->setCoords(-HIDE_POSITION, position);  // location of point 1 in plot coordinate
            _infLine_v.at(index).at(axis_tag_index)->point2->setCoords(+HIDE_POSITION, position);  // location of point 2 in plot coordinate
        }
    }
}

void CustomPlot::set_axis_tag_v(std::vector<double> axis_tag_v, size_t index) {
    if (index < this->_cat_v.size()) {
        uint8_t axis_tag_index = 0;
        for (auto &[position, cat] : zip(axis_tag_v, _cat_v[index])) {
            _position_v.at(index).at(axis_tag_index) = position;
            cat->updatePosition(position);
            ++axis_tag_index;
        }
        for (auto &[position, infLine] : zip(axis_tag_v, _infLine_v[index])) {
            infLine->point1->setCoords(-HIDE_POSITION, position);  // location of point 1 in plot coordinate
            infLine->point2->setCoords(+HIDE_POSITION, position);  // location of point 2 in plot coordinate
        }
    }
}

//void CustomPlot::data_size_changed(int size) {
//    Q_UNUSED(size);
//    //this->data_size = size;
//}

void CustomPlot::clear() {
    this->clear_all_graphs();
}

void CustomPlot::clear(size_t index) {
    this->clear_graph(index);
    this->clear_axis_tag(index);
    this->remove_graph(index);
}

void CustomPlot::clear_all() {
    this->clear_all_graphs();
    //this->clear_all_axis_tag(); //moved to clear_graph
    this->remove_all_graphs();

    this->axisRect()->axis(QCPAxis::atRight, 1)->setVisible(false);
    //this->graph_en.clear();
}

void CustomPlot::set_range(double x_upper, double x_lower, double y_upper, double y_lower, size_t index) {
    auto x_range = QCPRange(x_lower, x_upper);
    auto y_range = QCPRange(y_lower, y_upper);
    this->xAxis->setRange(x_range);
    if ((index % 2) == 0) {
        this->yAxis->setRange(y_range);
    } else {
        this->axisRect()->axis(QCPAxis::atRight, 1)->setRange(y_range);
    }
    this->replot(QCustomPlot::rpQueuedReplot);
    //qCDebug(logDebug) << x_range << y_range;
}
