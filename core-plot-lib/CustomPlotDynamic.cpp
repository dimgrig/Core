#include "CustomPlotDynamic.h"

#include <QMenu>
#include <QActionGroup>

#include "CustomAxisTag.h"
#include "CoreConfig.h"
#include "Logger.h"

CustomPlotDynamic::CustomPlotDynamic(const QString &objectName, QWidget *parent, CustomPlotOrientation orientation)
    : CustomPlotBase(objectName, parent)
{
    setObjectName("CustomPlotDynamic");
    setupUi();
    setupConnections();
}

CustomPlotDynamic::~CustomPlotDynamic() {

}

void CustomPlotDynamic::addGraphSpecificActions(QMenu *menu) {
    for (QCPGraph *g : _graphs) {
        if (!g->visible()) {
            menu->addAction(tr("Показать график \"%1\"").arg(g->name()),
                            [this, g]() {
                                g->setVisible(true);
                                syncLegendVisibility();
                                replot();
                            });
        }
    }
    if (selectedGraphs().size() > 0) {
        menu->addAction(tr("Скрыть выбранный график"), [this]() {
            auto sel = selectedGraphs();
            if (!sel.isEmpty()) {
                sel.first()->setVisible(false);
                syncLegendVisibility();
                replot();
            }
        });
    }
    if (!_graphs.isEmpty()) {
        menu->addAction(tr("Скрыть все графики"), [this]() {
            for (QCPGraph *g : _graphs) g->setVisible(false);
            syncLegendVisibility();
            replot();
        });
    }
    menu->addAction(tr("Очистить график"), [this]() {
        for (QCPGraph *g : _graphs) {
            g->data()->clear();
            g->rescaleAxes();
        }
        replot();
    });
}

// ---------- Внутренняя настройка ----------
void CustomPlotDynamic::setupUi() {
    // настройка графика как в CustomPlot, но без фиксированного числа графиков
    this->setOpenGl(true);
    this->setAntialiasedElements(QCP::aeNone);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                    QCP::iSelectLegend | QCP::iSelectPlottables);

    // легенда
    legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    legend->setFont(legendFont);
    legend->setSelectedFont(legendFont);
    legend->setSelectableParts(QCPLegend::spItems);
    move_legend((int)(Qt::AlignBottom | Qt::AlignRight));

    // оси
    axisRect()->setupFullAxesBox();

    QSharedPointer<QCPAxisTickerDateTime> dt_axis(new QCPAxisTickerDateTime);
    dt_axis->setDateTimeFormat(DATE_TIME_FORMAT_WITHOUT_MS_WRAP);
    xAxis->setTicker(dt_axis);

    // правая ось (активная по умолчанию)
    axisRect()->addAxis(QCPAxis::atRight);
    _activeRightAxis = axisRect()->axis(QCPAxis::atRight, 1);
    _activeRightAxis->setLabel("ADC");   // временная подпись
    _activeRightAxis->setVisible(false); // изначально скрыта

    // дополнительные правые оси будут добавляться через getOrCreateAxis

    xAxis->setLabel("Sample");
    yAxis->setLabel("Value");
}

void CustomPlotDynamic::setupConnections() {
    // контекстное меню
    connect(this, &CustomPlotBase::customContextMenuRequested, this, &CustomPlotDynamic::contextMenuRequested);

    // курсор и статистика для активной правой оси (изначально _activeRightAxis)
    if (_activeRightAxis) {
        connect(_activeRightAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
                this, &CustomPlotDynamic::onAxisRangeChanged);
    }
    // для левой оси (базовые соединения уже есть в CustomPlotBase? Нет, добавим)
    connect(yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
            this, &CustomPlotDynamic::onAxisRangeChanged);
    // для оси X
    connect(xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
            this, &CustomPlotDynamic::onAxisRangeChanged);
}

void CustomPlotDynamic::onAxisRangeChanged(const QCPRange &range) {
    Q_UNUSED(range);
    if (_cursorManager)
        _cursorManager->handleAxisRangeChanged();
    if (_statsManager && _statsManager->isVisible())
        _statsManager->updateStats();
}

// ---------- Работа с осями ----------
void CustomPlotDynamic::setLeftAxisUnit(const QString& unit) {
    _leftAxisUnit = unit;
    yAxis->setLabel(unit.isEmpty() ? tr("Value") : unit);
}

QString CustomPlotDynamic::leftAxisUnit() const {
    return _leftAxisUnit;
}

QCPAxis* CustomPlotDynamic::getOrCreateAxis(const QString& unit) {
    if (unit.isEmpty() || unit == _leftAxisUnit)
        return yAxis;

    auto it = _unitToAxis.find(unit);
    if (it != _unitToAxis.end())
        return it.value();

    // создаём новую правую ось
    return createRightAxis(unit);
}

QCPAxis* CustomPlotDynamic::createRightAxis(const QString& unit) {
    QCPAxis *axis = axisRect()->addAxis(QCPAxis::atRight);
    axis->setLabel(unit);
    axis->setVisible(true);
    _rightAxes.append(axis);
    _unitToAxis[unit] = axis;
    return axis;
}

// ---------- Графики ----------
QCPGraph* CustomPlotDynamic::createGraph(const QString& unit, const QString& name,
                                         const QString& styleKey, int colorIndex) {
    QCPAxis *valueAxis = getOrCreateAxis(unit);
    QCPGraph *graph = addGraph(xAxis, valueAxis);
    QString finalName = name.isEmpty() ? QString("Graph %1").arg(_graphs.size() + 1) : name;
    graph->setName(finalName);

    if (colorIndex < 0) colorIndex = _graphs.size();

    // Базовый цвет (будет переопределён стилем, если ключ задан)
    QColor color = this->defaultGraphColor(colorIndex);
    graph->setPen(QPen(color, this->defaultGraphWidth()));
    graph->selectionDecorator()->setPen(QPen(color.darker(), this->defaultGraphWidth()));
    graph->setLineStyle(this->defaultGraphLineStyle());

    if (!styleKey.isEmpty()) {
        graph->setProperty("styleKey", styleKey);
        graph->setProperty("colorIndex", colorIndex);
        applySavedStyle(graph, styleKey);   // загружает из QSettings и применяет поверх
    }

    _graphs.append(graph);
    _graphUnit[graph] = unit;
    if (valueAxis != yAxis)
        valueAxis->setVisible(true);

    syncLegendVisibility();
    replot(QCustomPlot::rpQueuedReplot);
    return graph;
}

void CustomPlotDynamic::removeGraph(QCPGraph* graph) {
    if (!graph || !_graphs.contains(graph))
        return;

    QCPAxis *axis = graph->valueAxis();
    QString unit = _graphUnit.value(graph);

    // Удаляем из QCustomPlot (метод базового класса) – квалифицированный вызов
    QCustomPlot::removeGraph(graph);
    _graphs.removeOne(graph);
    _graphUnit.remove(graph);

    // Проверяем, остались ли графики на этой оси
    bool hasGraphs = false;
    for (QCPGraph *g : _graphs) {
        if (g->valueAxis() == axis) {
            hasGraphs = true;
            break;
        }
    }
    if (!hasGraphs && axis != yAxis && axis != _activeRightAxis) {
        _unitToAxis.remove(unit);
        _rightAxes.removeOne(axis);
        axisRect()->removeAxis(axis);
    } else if (!hasGraphs && axis == _activeRightAxis) {
        axis->setVisible(false);
    }

    syncLegendVisibility();
    replot(QCustomPlot::rpQueuedReplot);
}

void CustomPlotDynamic::removeAllGraphs() {
    // 1. Удаляем все графики (из QCustomPlot и внутренних списков)
    QList<QCPGraph*> graphsToRemove = _graphs;
    for (QCPGraph *g : graphsToRemove)
        QCustomPlot::removeGraph(g);
    _graphs.clear();
    _graphUnit.clear();

    // 2. Удаляем все правые оси, кроме активной (_activeRightAxis)
    for (auto it = _unitToAxis.begin(); it != _unitToAxis.end(); ) {
        QCPAxis *axis = it.value();
        if (axis != _activeRightAxis) {
            axisRect()->removeAxis(axis);
            it = _unitToAxis.erase(it);
        } else {
            ++it;
        }
    }

    // 3. Очищаем базовый список правых осей
    _rightAxes.clear();

    // 4. Обновляем легенду и перерисовываем
    syncLegendVisibility();
    replot(QCustomPlot::rpQueuedReplot);
}

QCPGraph* CustomPlotDynamic::graphByName(const QString& name) const {
    for (QCPGraph *g : _graphs) {
        if (g->name() == name)
            return g;
    }
    return nullptr;
}

void CustomPlotDynamic::setGraphData(QCPGraph* graph, const QVector<double> &values, const QVector<double> &ticks) {
    if (!graph || !_graphs.contains(graph))
        return;
    graph->setData(ticks, values, true);
    graph->rescaleAxes();
    replot(QCustomPlot::rpQueuedReplot);
}
