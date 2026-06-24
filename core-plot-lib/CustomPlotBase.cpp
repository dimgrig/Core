#include "CustomPlotBase.h"

CustomPlotBase::CustomPlotBase(QWidget *parent) :
    QCustomPlot(parent)
{
    this->setWindowFlags(Qt::Window);

    this->setMinimumSize(QSize(200, 200));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->setSizePolicy(sizePolicy);
    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, false);

    //this->axisRect()->setMargins(QMargins(100, 100, 100, 100));

    if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
        this->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(75); //must be for axis tag
    }

    connect(this, &CustomPlotBase::selectionChangedByUser, this, &CustomPlotBase::selection_changed);

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(this, &CustomPlotBase::mousePress, this, &CustomPlotBase::mouse_press);
    connect(this, &CustomPlotBase::mouseWheel, this, &CustomPlotBase::mouse_wheel);

    connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this->xAxis2, SLOT(setRange(QCPRange)));
    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2, SLOT(setRange(QCPRange)));

    {
        _cursorManager = std::make_unique<CursorManager>(this);

        _statsManager = std::make_unique<StatsManager>(this);
        _statsManager->setVisible(false);

        connect(this->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](){ _cursorManager->handleAxisRangeChanged(); });
        connect(this->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](){ _cursorManager->handleAxisRangeChanged(); });
        if (axisRect()->axis(QCPAxis::atRight, 1)) {
            connect(axisRect()->axis(QCPAxis::atRight, 1), QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](){ _cursorManager->handleAxisRangeChanged(); });
        }

        auto updateStatsLambda = [this]() {
            updateStatsIfVisible();
        };

        connect(xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, updateStatsLambda);
        connect(yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, updateStatsLambda);
        connect(xAxis2, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, updateStatsLambda);
        connect(yAxis2, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, updateStatsLambda);

        // Правая ось, если используется (как в вашем коде)
        if (auto rightAxis = axisRect()->axis(QCPAxis::atRight, 1)) {
            connect(rightAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, updateStatsLambda);
        }
    }
}

CustomPlotBase::~CustomPlotBase() {
    _statsManager.reset();
    _cursorManager.reset();
}

void CustomPlotBase::export_data(QTextStream &stream) {
    QLocale locale;
    QStringList headers;
    headers << this->xAxis->label() << this->yAxis->label();
    QString headers_row = headers.join(ELEMENTS_DELIMETER);
    headers_row += "\r\n";
    stream << headers_row;
    for (int index = 0; index < this->graphCount(); ++index) {
        for (const auto &d : *this->graph(index)->data()) {
            QStringList sl;
#if EXPORT_KEY_TIME
            sl << QDateTime::fromTime_t(d.key).toString(DT_FORMAT_WITHOUT_MS);
#else
            sl << locale.toString(d.key);
#endif
            sl << locale.toString(d.value);

            QString row = sl.join(ELEMENTS_DELIMETER);
            row += "\r\n";
            stream << row;
        }
    }
}

void CustomPlotBase::sync_selection() {
    for (int index = 0; index < this->graphCount(); ++index) {
        QCPGraph *graph = this->graph(index);
        QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected()) {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}

void CustomPlotBase::setCursorMode(CursorManager::Mode mode) {
    _cursorManager->setMode(mode);
}

CursorManager::Mode CustomPlotBase::cursorMode() const {
    return _cursorManager->mode();
}

void CustomPlotBase::setStatsVisible(bool visible) {
    _statsManager->setVisible(visible);
}

bool CustomPlotBase::statsVisible() const {
    return _statsManager->isVisible();
}

void CustomPlotBase::setStatsOnlyVisibleRange(bool onlyVisible) {
    _statsManager->setShowOnlyVisibleRange(onlyVisible);
}

bool CustomPlotBase::statsOnlyVisibleRange() const {
    return _statsManager->showOnlyVisibleRange();
}

void CustomPlotBase::updateStats() {
    _statsManager->updateStats();
}

void CustomPlotBase::updateStatsIfVisible () {
    if (_statsManager && _statsManager->isVisible())
        _statsManager->updateStats();
}

void CustomPlotBase::syncLegendVisibility() {
    for (int i = 0; i < graphCount(); ++i) {
        QCPGraph *g = graph(i);
        QCPPlottableLegendItem *item = legend->itemWithPlottable(g);
        if (item)
            item->setVisible(g->visible());
    }

    // Принудительно пересчитываем размер легенды на основе видимых элементов
    legend->update(QCPLayoutElement::upLayout);
    // Если легенда размещена через insetLayout (move_legend), обновляем и его
    if (axisRect()->insetLayout()) {
        axisRect()->insetLayout()->update(QCPLayoutElement::upLayout);
    }
}

void CustomPlotBase::move_legend(int pos) {
    this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)pos);
    this->replot();
}

// void CustomPlotBase::mouse_press() {
//     // if an axis is selected, only allow the direction of that axis to be dragged
//     // if no axis is selected, both directions may be dragged
//     if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
//         this->axisRect()->setRangeDrag(this->xAxis->orientation());
//     } else if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeDragAxes(this->xAxis2, this->yAxis);
//         this->axisRect()->setRangeDrag(this->yAxis->orientation());
//     } else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
//         this->axisRect()->setRangeDrag(this->yAxis->orientation());
//     } else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis2);
//         this->axisRect()->setRangeDrag(this->yAxis2->orientation());
//     } else if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
//         if (this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeDragAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
//         this->axisRect()->setRangeDrag(this->axisRect()->axis(QCPAxis::atRight, 1)->orientation());
// //    } else if (this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spAxis)) {
// //        this->axisRect()->setRangeDragAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 2));
// //        this->axisRect()->setRangeDrag(this->axisRect()->axis(QCPAxis::atRight, 2)->orientation());
// //    } else if (this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spAxis)) {
// //        this->axisRect()->setRangeDragAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 3));
// //        this->axisRect()->setRangeDrag(this->axisRect()->axis(QCPAxis::atRight, 3)->orientation());
//         } else {
//             this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
//             this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
//         }
//     } else {
//         this->axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
//         this->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
//     }
// }

// void CustomPlotBase::mouse_wheel()
// {
//     // if an axis is selected, only allow the direction of that axis to be zoomed
//     // if no axis is selected, both directions may be zoomed

//     if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
//         this->axisRect()->setRangeZoom(this->xAxis->orientation());
//     } else if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeDragAxes(this->xAxis2, this->yAxis);
//         this->axisRect()->setRangeDrag(this->yAxis->orientation());
//     } else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
//         this->axisRect()->setRangeZoom(this->yAxis->orientation());
//     } else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis2);
//         this->axisRect()->setRangeZoom(this->yAxis2->orientation());
//     } else if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
//         if (this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spAxis)) {
//         this->axisRect()->setRangeZoomAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 1));
//         this->axisRect()->setRangeZoom(this->axisRect()->axis(QCPAxis::atRight, 1)->orientation());
// //    } else if (this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spAxis)) {
// //        this->axisRect()->setRangeZoomAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 2));
// //        this->axisRect()->setRangeZoom(this->axisRect()->axis(QCPAxis::atRight, 2)->orientation());
// //    } else if (this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spAxis)) {
// //        this->axisRect()->setRangeZoomAxes(this->xAxis, this->axisRect()->axis(QCPAxis::atRight, 3));
// //        this->axisRect()->setRangeZoom(this->axisRect()->axis(QCPAxis::atRight, 3)->orientation());
//         } else {
//             this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
//             this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
//         }
//     } else {
//         this->axisRect()->setRangeZoomAxes(this->xAxis, this->yAxis);
//         this->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
//     }
// }

// void CustomPlotBase::selection_changed() {
//     // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
//     if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
//         this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
//         this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
//         this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
//         this->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//         this->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//     }

//     // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
//     if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
//         this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
//         this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
//         this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)    ) {
//         this->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//         this->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//     }

//     if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
//         if (this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spAxis) ||
//             this->axisRect()->axis(QCPAxis::atRight, 1)->selectedParts().testFlag(QCPAxis::spTickLabels)) {
//             this->axisRect()->axis(QCPAxis::atRight, 1)->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//     //    } else if (this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spAxis) ||
//     //               this->axisRect()->axis(QCPAxis::atRight, 2)->selectedParts().testFlag(QCPAxis::spTickLabels)) {
//     //        this->axisRect()->axis(QCPAxis::atRight, 2)->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//     //    } else if (this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spAxis) ||
//     //               this->axisRect()->axis(QCPAxis::atRight, 3)->selectedParts().testFlag(QCPAxis::spTickLabels)) {
//     //        this->axisRect()->axis(QCPAxis::atRight, 3)->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
//         }
//     }

//     // synchronize selection of graphs with selection of corresponding legend items:
//     this->sync_selection();
// }

void CustomPlotBase::mouse_press() {
    QCPAxis *selectedAxis = nullptr;

    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = xAxis;
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = yAxis;
    else if (xAxis2->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = xAxis2;
    else if (yAxis2->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = yAxis2;
    else {
        for (QCPAxis *ax : _rightAxes) {
            if (ax->selectedParts().testFlag(QCPAxis::spAxis)) {
                selectedAxis = ax;
                break;
            }
        }
    }

    if (selectedAxis) {
        if (selectedAxis->orientation() == Qt::Horizontal) {
            axisRect()->setRangeDragAxes(selectedAxis, nullptr);
        } else {
            axisRect()->setRangeDragAxes(nullptr, selectedAxis);
        }
        axisRect()->setRangeDrag(selectedAxis->orientation());
    } else {
        axisRect()->setRangeDragAxes(xAxis, yAxis);
        axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
}

void CustomPlotBase::mouse_wheel() {
    QCPAxis *selectedAxis = nullptr;

    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = xAxis;
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = yAxis;
    else if (xAxis2->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = xAxis2;
    else if (yAxis2->selectedParts().testFlag(QCPAxis::spAxis))
        selectedAxis = yAxis2;
    else {
        for (QCPAxis *ax : _rightAxes) {
            if (ax->selectedParts().testFlag(QCPAxis::spAxis)) {
                selectedAxis = ax;
                break;
            }
        }
    }

    if (selectedAxis) {
        if (selectedAxis->orientation() == Qt::Horizontal) {
            axisRect()->setRangeZoomAxes(selectedAxis, nullptr);
        } else {
            axisRect()->setRangeZoomAxes(nullptr, selectedAxis);
        }
        axisRect()->setRangeZoom(selectedAxis->orientation());
    } else {
        axisRect()->setRangeZoomAxes(xAxis, yAxis);
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }
}

void CustomPlotBase::selection_changed() {
    // X оси синхронизируются как прежде
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    // Y оси синхронизируются: левая и все правые независимы
    if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
        yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    for (QCPAxis *ax : _rightAxes) {
        if (ax->selectedParts().testFlag(QCPAxis::spAxis) ||
            ax->selectedParts().testFlag(QCPAxis::spTickLabels)) {
            ax->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        }
    }

    sync_selection();
}

void CustomPlotBase::mouseMoveEvent(QMouseEvent *event) {
    QCustomPlot::mouseMoveEvent(event); // вызываем базовый для стандартной обработки
    _cursorManager->handleMouseMove(event->pos());
}

void CustomPlotBase::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::MiddleButton:
            this->yAxis->rescale();
            this->yAxis2->rescale();
            this->xAxis->rescale();
            for (QCPAxis *ax : _rightAxes) {
                ax->rescale();
            }
            // if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
            //     this->axisRect()->axis(QCPAxis::atRight, 1)->rescale();
            // }
            this->replot();
            return;
            break;
        default:

            break;
    }

    QCustomPlot::mousePressEvent(event);
}

void CustomPlotBase::set_legend_state(bool state) {
    this->_legend_state = state;
    this->legend->setVisible(state);
    this->replot();
}

void CustomPlotBase::rescale(int index) {
    if (this->_autorescale) {
        //this->rescaleAxes();
        this->xAxis->rescale();
        if ((index % 2) == 0) {
            this->yAxis->rescale();
        } else {
            if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
                this->axisRect()->axis(QCPAxis::atRight, 1)->rescale();
            }
        }
    }
    this->replot(QCustomPlot::rpQueuedReplot); //this->replot(); //
}

void CustomPlotBase::autoscale_all() {
    if (this->_autorescale)
        this->_autorescale = 0;
    else
        this->_autorescale = 1;
}

void CustomPlotBase::export_csv() {
    QString dir = "График " + QDateTime::currentDateTime().toString(DT_FORMAT_WITHOUT_MS) + ".csv"; //cfg.TEST_RESULT_DIR +
    QString filename = QFileDialog::getSaveFileName(this,
            tr("%1 - Сохранить").arg(QApplication::applicationDisplayName()),
            dir,
            tr("(*.csv)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);

    if (filename != "") {
        QScopedPointer<QFile> file(new QFile(filename));
        if (file.data()->open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream stream(file.data());
            this->export_data(stream);
            file->close();
        }
    }
}

void CustomPlotBase::export_png() {
    QString dir = "График " + QDateTime::currentDateTime().toString(DT_FORMAT_WITHOUT_MS) + ".png"; //cfg.TEST_RESULT_DIR +
    QString filename = QFileDialog::getSaveFileName(this,
            tr("%1 - Сохранить").arg(QApplication::applicationDisplayName()),
            dir,
            tr("(*.png)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);

    if (filename != "") {
        this->savePng(filename);
    }
}

void CustomPlotBase::contextMenuRequested(QPoint pos) {
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Если клик по легенде
    if (legend->selectTest(pos, false) >= 0) {
        menu->addAction(tr("Верхний левый угол"),  [this](){ move_legend((int)(Qt::AlignTop|Qt::AlignLeft)); });
        menu->addAction(tr("Вверху по центру"),    [this](){ move_legend((int)(Qt::AlignTop|Qt::AlignHCenter)); });
        menu->addAction(tr("Верхний правый угол"), [this](){ move_legend((int)(Qt::AlignTop|Qt::AlignRight)); });
        menu->addAction(tr("Нижний правый угол"),  [this](){ move_legend((int)(Qt::AlignBottom|Qt::AlignRight)); });
        menu->addAction(tr("Вниз по центру"),      [this](){ move_legend((int)(Qt::AlignBottom|Qt::AlignHCenter));});
        menu->addAction(tr("Нижний левый угол"),   [this](){ move_legend((int)(Qt::AlignBottom|Qt::AlignLeft)); });
        // Дополнительно: показать/скрыть легенду
        if (get_legend_state()) {
            menu->addAction(tr("Скрыть легенду"), [this](){ set_legend_state(false); });
        } else {
            menu->addAction(tr("Показать легенду"), [this](){ set_legend_state(true); });
        }
    } else {
        // Автомасштаб
        if (get_autorescale()) {
            menu->addAction(tr("Выключить автомасштаб"), [this](){ set_autorescale(false); });
        } else {
            menu->addAction(tr("Включить автомасштаб"), [this](){ set_autorescale(true); });
        }

        // Специфические для наследников пункты
        addGraphSpecificActions(menu);

        // Общие подменю
        // Курсор
        QMenu *cursorMenu = menu->addMenu(tr("Курсор"));
        QAction *actNo = cursorMenu->addAction(tr("Выключен"));
        QAction *actVert = cursorMenu->addAction(tr("Вертикальный"));
        QAction *actHor = cursorMenu->addAction(tr("Горизонтальный"));
        QAction *actCross = cursorMenu->addAction(tr("Перекрестие"));
        actNo->setCheckable(true); actVert->setCheckable(true);
        actHor->setCheckable(true); actCross->setCheckable(true);
        QActionGroup *cursorGroup = new QActionGroup(cursorMenu);
        cursorGroup->addAction(actNo); cursorGroup->addAction(actVert);
        cursorGroup->addAction(actHor); cursorGroup->addAction(actCross);
        switch (_cursorManager->mode()) {
        case CursorManager::Mode::NoCursor: actNo->setChecked(true); break;
        case CursorManager::Mode::VerticalCursor: actVert->setChecked(true); break;
        case CursorManager::Mode::HorizontalCursor: actHor->setChecked(true); break;
        case CursorManager::Mode::CrossCursor: actCross->setChecked(true); break;
        }
        connect(actNo, &QAction::triggered, [this](){ setCursorMode(CursorManager::Mode::NoCursor); });
        connect(actVert, &QAction::triggered, [this](){ setCursorMode(CursorManager::Mode::VerticalCursor); });
        connect(actHor, &QAction::triggered, [this](){ setCursorMode(CursorManager::Mode::HorizontalCursor); });
        connect(actCross, &QAction::triggered, [this](){ setCursorMode(CursorManager::Mode::CrossCursor); });

        // Статистика
        QMenu *statsMenu = menu->addMenu(tr("Статистика"));
        QAction *actStatsOn = statsMenu->addAction(tr("Показывать статистику"));
        actStatsOn->setCheckable(true);
        actStatsOn->setChecked(statsVisible());
        QAction *actStatsVisible = statsMenu->addAction(tr("Только по видимой области"));
        actStatsVisible->setCheckable(true);
        actStatsVisible->setChecked(statsOnlyVisibleRange());
        connect(actStatsOn, &QAction::triggered, [this](bool on){ setStatsVisible(on); });
        connect(actStatsVisible, &QAction::triggered, [this](bool on){ setStatsOnlyVisibleRange(on); });

        // Экспорт
        QMenu *exportMenu = menu->addMenu(tr("Экспорт"));
        exportMenu->addAction(tr("Экспортировать график в csv"), this, &CustomPlotBase::export_csv);
        exportMenu->addAction(tr("Экспортировать график в png"), this, &CustomPlotBase::export_png);
    }
    menu->popup(mapToGlobal(pos));
}
