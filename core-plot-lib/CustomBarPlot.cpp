#include "CustomBarPlot.h"

#include "CoreConfig.h"
#include "Logger.h"

CustomBarPlot::CustomBarPlot(QWidget *parent, CustomPlotOrientation orientation, size_t bar_number, bool text_x_ticker) :
        CustomPlotBase(parent),
        _orientation(orientation),
        _bar_number(bar_number),
        _text_x_ticker(text_x_ticker)
{
    form_charts();

//    this->setPlottingHint(QCP::PlottingHint::phFastPolylines, true);
//    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, true);

    connect(this, &CustomBarPlot::customContextMenuRequested, this, &CustomBarPlot::context_menu_request);
}

CustomBarPlot::~CustomBarPlot() {

}

void CustomBarPlot::export_data(QTextStream &stream) {
    QLocale locale;
    QStringList tick_labels_sl;
    for (const auto &label : this->xAxis->tickVectorLabels()) {
        tick_labels_sl << label;
    }
    QStringList headers;
    if (tick_labels_sl.size() > 0) {
        headers << "№" << this->xAxis->label() << this->yAxis->label();
    } else {
        headers << this->xAxis->label() << this->yAxis->label();
    }
    QString headers_row = headers.join(ELEMENTS_DELIMETER);
    headers_row += "\r\n";
    stream << headers_row;
    for (unsigned int index = 0; index < this->bar_v.size(); ++index) {
        int i = 0;
        for (const auto &d : *this->bar_v[index]->data()) {
            QStringList sl;
            sl << locale.toString(d.key);
            if (tick_labels_sl.size() > i) {
                sl << tick_labels_sl.at(i);
            }
            sl << locale.toString(d.value);

            QString row = sl.join(ELEMENTS_DELIMETER);
            row += "\r\n";
            stream << row;
            ++i;
        }
    }
}

void CustomBarPlot::sync_selection() {
    for (unsigned int index = 0; index < this->bar_v.size(); ++index) {
        QCPBars *bar = this->bar_v[index];
        QCPPlottableLegendItem *item = this->legend->itemWithPlottable(bar);
        if (item->selected() || bar->selected()) {
            item->setSelected(true);
            bar->setSelection(QCPDataSelection(bar->data()->dataRange()));
        }
    }
}

void CustomBarPlot::form_charts() {
    //this->setOpenGl(true);
    QCustomPlot::setOpenGl(true);
    QCustomPlot::setAntialiasedElements(QCP::aeNone);

    for (size_t i = 0; i < _bar_number; ++i) {
        if (this->_orientation == OrientationHorizontal) {
            bar_names.push_back("");
        }
        if (this->_orientation == OrientationVertical) {
            bar_names.push_back("");
        }

        bar_creator[bar_names.at(i)] = std::bind(&CustomBarPlot::add_bar, this, i);
    }

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
    this->move_legend((int)(Qt::AlignTop|Qt::AlignRight)); //Qt::AlignBottom|Qt::AlignLeft

    //top axes
    this->axisRect()->setupFullAxesBox();

    // configure bottom axis to show date-time
//    QSharedPointer<QCPAxisTickerDateTime> dt_axis(new QCPAxisTickerDateTime);
//    dt_axis->setDateTimeFormat(DT_FORMAT_SHORT);//DT_FORMAT_SHORT
//    this->xAxis->setTicker(dt_axis);

    //right axe
//    this->axisRect()->addAxis(QCPAxis::atRight);
//    this->axisRect()->axis(QCPAxis::atRight, 1)->setVisible(false);
//    this->axisRect()->axis(QCPAxis::atRight, 1)->setLabel("");

//    //right axe for temp with tag
//    this->yAxis2->setVisible(true);
//    this->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(100); // add some padding to have space for tags
//    this->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(50); // add some padding to have space for tags
//    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2, SLOT(setRange(QCPRange))); // left axis only mirrors inner right axis

    if (_text_x_ticker) {
        QSharedPointer<QCPAxisTickerText> textXTicker(new QCPAxisTickerText);
        this->xAxis->setTicker(textXTicker);
        QFont textXTickerFont = font();
        textXTickerFont.setPointSize(8);
        this->xAxis->setTickLabelFont(textXTickerFont);

        this->xAxis->setTickLabelRotation(-90);
        this->xAxis->setTickLabelSide(QCPAxis::lsInside);
    } else {
        QSharedPointer<QCPAxisTickerFixed> fixedXTicker(new QCPAxisTickerFixed);
        fixedXTicker->setTickStepStrategy(QCPAxisTickerFixed::tssReadability);
        fixedXTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples); // and no scaling of the tickstep (like multiples or powers) is allowed
        this->xAxis->setTicker(fixedXTicker);
    }

    QSharedPointer<QCPAxisTickerFixed> fixedYTicker(new QCPAxisTickerFixed);
    //fixedYTicker->setTickStep(1.0); // tick step shall be 1.0
    //fixedYTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone); // and no scaling of the tickstep (like multiples or powers) is allowed
    fixedYTicker->setTickStepStrategy(QCPAxisTickerFixed::tssReadability);
    fixedYTicker->setScaleStrategy(QCPAxisTickerFixed::ssMultiples); // and no scaling of the tickstep (like multiples or powers) is allowed
    this->yAxis->setTicker(fixedYTicker);

    this->xAxis->setSubTicks(false);
    this->xAxis->grid()->setSubGridVisible(false);
    this->yAxis->setSubTicks(false);
    this->yAxis->grid()->setSubGridVisible(false);

    for (size_t i = 0; i < _bar_number; ++i) {
        this->add_bar(i);
    }

    // give the axes some labels:
    this->xAxis->setLabel("");
    this->yAxis->setLabel("");
}

void CustomBarPlot::add_bar(size_t index) {
    if (_orientation == OrientationVertical) {
        bar_v.push_back(new QCPBars(this->yAxis, this->xAxis));
    }
    if (_orientation == OrientationHorizontal) {
        if ((index % 2) == 0) {
            bar_v.push_back(new QCPBars(this->xAxis, this->yAxis));
        } else {
            //bar_v.push_back(new QCPBars(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1)));
        }
    }
//    qCDebug(logDebug) << this->legend->itemCount();
//    if (bar_v.size() > index) {
//        qCDebug(logDebug) << bar_v.at(index)->name();
//        auto plot = bar_v.at(index)->parentPlot();
//        if (plot) {
//            qCDebug(logDebug) << bar_v.at(index)->parentPlot()->objectName();
//        }
//        bar_v.at(index)->addToLegend();
//    }
//    QString PEN_COLOR = G_COLOR;
//    QString SELECTION_PEN_COLOR = DG_COLOR;
//    QString BRUSH_COLOR = G_COLOR;
//    if (index < PEN_COLOR_v.size()) {
//        PEN_COLOR = PEN_COLOR_v.at(index);
//        SELECTION_PEN_COLOR = SELECTION_PEN_COLOR_v.at(index);
//        BRUSH_COLOR = PEN_COLOR_v.at(index);
//    }
    QString PEN_COLOR = R_COLOR;
    QString SELECTION_PEN_COLOR = DR_COLOR;
    QString BRUSH_COLOR = R_COLOR;
    bar_v.at(index)->setName(bar_names.at(index));
    bar_v.at(index)->setPen(QPen(((QColor)PEN_COLOR).lighter(50), this->get_width()));
    bar_v.at(index)->setBrush(((QColor)BRUSH_COLOR));
    bar_v.at(index)->selectionDecorator()->setPen(QPen((QColor)SELECTION_PEN_COLOR, 1));
    bar_en[bar_names.at(index)] = true;
}

void CustomBarPlot::set_labels(QString yAxisLabel, QString xAxisLabel) {
    this->yAxis->setLabel(yAxisLabel);
    this->xAxis->setLabel(xAxisLabel);
}

void CustomBarPlot::set_cp_name(QString name, size_t index) {
    if (index < bar_v.size()) {
        bar_v[index]->setName(name);
        bar_names[index] = name;
        bar_en[bar_names[index]] = true;
    }
}

void CustomBarPlot::set_x_ticks(QVector<double> ticks, QVector<QString> labels) {
    if (_text_x_ticker) {
        QSharedPointer<QCPAxisTickerText> textXTicker(new QCPAxisTickerText);
        textXTicker->addTicks(ticks, labels);
        this->xAxis->setTicker(textXTicker);
    }
}

void CustomBarPlot::set_y_tick_step(double max_y_value) {
    Q_UNUSED(max_y_value);
//    double tick_step = 1;
//    if (max_y_value < 10) {
//        tick_step = 1;
//    } else if (max_y_value < 100) {
//        tick_step = 10;
//    } else if (max_y_value < 1000) {
//        tick_step = 100;
//    } else if (max_y_value < 10000) {
//        tick_step = 1000;
//    } else if (max_y_value < 100000) {
//        tick_step = 10000;
//    } else if (max_y_value < 1000000) {
//        tick_step = 100000;
//    } else if (max_y_value < 10000000) {
//        tick_step = 1000000;
//    }
//    dynamic_cast<QCPAxisTickerFixed*>(&*this->yAxis->ticker())->setTickStep(tick_step);
}

void CustomBarPlot::remove_selected_bar() {
    if (!this->selectedGraphs().empty()) {
        this->legend->selectedItems().first()->setVisible(false);
        this->bar_en[this->selectedGraphs().first()->name()] = false;
        this->selectedGraphs().first()->setVisible(false);

        this->replot(QCustomPlot::rpQueuedReplot); //this->replot();
    }
    if (!this->selectedPlottables().empty()) {
        this->legend->itemWithPlottable(this->selectedPlottables().first())->setVisible(false);
        this->bar_en[this->selectedPlottables().first()->name()] = false;
        this->selectedPlottables().first()->setVisible(false);

        this->replot(QCustomPlot::rpQueuedReplot); //this->replot();
    }
}

void CustomBarPlot::remove_bar(size_t index) {
    if (index < this->bar_v.size()) {
        this->bar_v[index]->setVisible(false);
        this->bar_en[bar_names[index]] = false;
        this->legend->item(index)->setVisible(false);
        this->bar_names[index].clear();
    }
    this->replot(QCustomPlot::rpQueuedReplot);
}

void CustomBarPlot::remove_all_bars() {
    for (unsigned int index = 0; index < this->bar_v.size(); index++) {
        remove_bar(index);
    }
}

void CustomBarPlot::show_all_bars() {
    for (unsigned int i = 0; i < this->bar_v.size(); i++) {
        this->bar_v[i]->setVisible(true);
        this->bar_en[bar_names[i]] = true;
        this->legend->item(i)->setVisible(true);
    }
}

void CustomBarPlot::show_bar(size_t index) {
    if (index < this->bar_v.size()) {
        this->bar_v[index]->setVisible(true);
        this->bar_en[bar_names[index]] = true;
        this->legend->item(index)->setVisible(true);
        if (_orientation == OrientationHorizontal) {
//            if ((index % 2) == 1) {
//                this->axisRect()->axis(QCPAxis::atRight, 1)->setVisible(true);
//            }
        }
        this->rescale(index);
    }
}

void CustomBarPlot::show_bar(const QString name) {
    for (unsigned int index = 0; index < this->bar_v.size(); index++) {
        if (name == bar_names[index]) {
            this->bar_v[index]->setVisible(true);
            this->bar_en[bar_names[index]] = true;
            this->legend->item(index)->setVisible(true);
            if (_orientation == OrientationHorizontal) {
//                if ((index % 2) == 1) {
//                    this->axisRect()->axis(QCPAxis::atRight, 1)->setVisible(true);
//                }
            }
        }
        //this->replot(QCustomPlot::rpQueuedReplot); //in rescale
        this->rescale(index);
    }
}

void CustomBarPlot::clear_bar(size_t index) {
    if (index < this->bar_v.size()) {
        this->bar_v[index]->data()->clear();
        //this->rescale(index); // no need?
        this->bar_v[index]->rescaleAxes();
        this->replot(QCustomPlot::rpQueuedReplot);
    }
}

void CustomBarPlot::clear_all_bars() {
    for(unsigned int index = 0; index < this->bar_v.size(); ++index) {
        this->clear_bar(index);
    }
}

//void CustomBarPlot::set_cp_data(const CustomPlotData& data, size_t index) {
//    //Profiler pr("CP set_cp_data");

//    if (index < graph_v.size()) {
//        graph_v[index]->data()->add(data._v);
//    }

//    this->show_graph(index);
//    if (this->visibleRegion() != QRegion()) {
//        this->rescale(index);
//    }
//}

//void CustomBarPlot::set_cp_data(std::vector<double> &data, std::vector<double> &ticks, size_t index) {
//    //Profiler pr("CP SCPD"); //12 ms
//    {
////        QVector<double> keys;
////        QVector<double> values;
////        {
////            Profiler pr("CP SCPD QVector"); // 8 ms
////            for(const auto &[value, key] : zip(data, ticks)) {
////                keys.push_back(key);
////                values.push_back(value);
////            }
////        }
//        if (index < graph_v.size()) {
//            //Profiler pr1("CP SCPD setData"); // 12 ms QVector and 12 mcs setData
//            graph_v[index].data()->setData(QVector(ticks.begin(), ticks.end()), QVector(data.begin(), data.end()), true);
//        }
//    }
//    {
//        //Profiler pr2("CP SCPD show_graph"); //26 mcs
//        this->show_graph(index);
//    }
////    if (this->visibleRegion() != QRegion()) { //rescale after add axis_tag
////        this->rescale(index);
////    }
//}

void CustomBarPlot::set_cp_data(QVector<double> &keys, QVector<double> &values, size_t index, bool add) {
    Q_UNUSED(add);
    //Profiler pr("CP SCPD"); //1-2 ms
//    {
//        if (index < graph_v.size()) {
//            if (add) {
//                double last_tick = (graph_v[index]->data()->end() - 1)->key;
//                //qCDebug(logDebug) << last_tick;
//                for(double&keys : ticks) {
//                    tick += last_tick;
//                }
//                graph_v[index].data()->addData(keys, values, true);
//            } else {
//                graph_v[index].data()->setData(keys, values, true);
//            }
//        }
//    }
    bar_v[index].data()->setData(keys, values, true);
    this->show_bar(index);
}

// void CustomBarPlot::get_cp_data(QVector<int> &tests_v, QVector<double> &tests_failed_v, size_t index) {
//     auto data = bar_v[index].data()->data();
//     for (auto it = data->begin(); it < data->end(); ++it) {
//         //tests_v.push_back((*it).mainKey()); //number 1 to N - not test numbers
//         tests_failed_v.push_back((*it).mainValue());
//     }
//     //tests_failed_v = ;
//     for (const auto &label : this->xAxis->tickVectorLabels()) {
//         tests_v.push_back(TestType_from_text(label.toStdString().c_str()));
//     }
//     //qCDebug(logDebug) << tests_v << tests_failed_v;
// }

void CustomBarPlot::clear() {
    this->clear_all_bars();
}

void CustomBarPlot::clear(size_t index) {
    this->clear_bar(index);
    this->remove_bar(index);
}

void CustomBarPlot::clear_all() {
    this->clear_all_bars();
    this->remove_all_bars();

    //this->axisRect()->axis(QCPAxis::atRight, 1)->setVisible(false);
    this->bar_en.clear();
}

void CustomBarPlot::context_menu_request(QPoint pos) {
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
        for (auto [name, en] : bar_en) {
            if (!en) {
                menu->addAction("Показать график \"" + name + "\"", [this, name](){
                    this->show_bar(name);
                });
            }
        }
        if (this->selectedGraphs().size() > 0)
            menu->addAction("Скрыть выбранный график", this, &CustomBarPlot::remove_selected_bar);
        if (this->selectedPlottables().size() > 0)
            menu->addAction("Скрыть выбранный график", this, &CustomBarPlot::remove_selected_bar);
        if (this->bar_v.size() > 0)
            menu->addAction("Скрыть все графики", this, &CustomBarPlot::remove_all_bars);
        menu->addAction("Очистить график", this, &CustomBarPlot::clear_all_bars);
        menu->addAction("Экспортировать график в csv", this, &CustomBarPlot::export_csv);
        menu->addAction("Экспортировать график в png", this, &CustomBarPlot::export_png);
//        if (this->autorescale)
//            menu->addAction("Выключить автомасштаб", this, &CustomPlotTUI::autoscale_all_graphs);
//        else
//            menu->addAction("Включить автомасштаб", this, &CustomPlotTUI::autoscale_all_graphs);
    }
    menu->popup(this->mapToGlobal(pos));
}





