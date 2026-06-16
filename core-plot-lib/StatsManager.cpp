#include "StatsManager.h"
#include "CustomPlotBase.h"
#include <QDebug>
#include <cmath>

StatsManager::StatsManager(CustomPlotBase *plot)
    : _plot(plot)
{
    createTextItem();
}

StatsManager::~StatsManager()
{
    destroyTextItem();
}

void StatsManager::setVisible(bool visible)
{
    if (_visible == visible)
        return;
    _visible = visible;
    if (_visible) {
        updateStats();
        if (_statsText)
            _statsText->setVisible(true);
    } else {
        if (_statsText)
            _statsText->setVisible(false);
    }
    if (_plot)
        _plot->replot();
}

void StatsManager::setShowOnlyVisibleRange(bool onlyVisible)
{
    _onlyVisibleRange = onlyVisible;
    if (_visible)
        updateStats();
}

void StatsManager::createTextItem()
{
    if (!_plot)
        return;

    if (!_statsText) {
        _statsText = new QCPItemText(_plot);
        _statsText->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
        _statsText->position->setType(QCPItemPosition::ptAbsolute);
        _statsText->position->setCoords(100, 20); // отступы от левого верхнего угла
        _statsText->setFont(QFont("monospace", 8));
        _statsText->setColor(Qt::black);
        _statsText->setBrush(QBrush(QColor(255, 255, 255, 200))); // полупрозрачный фон
        _statsText->setPadding(QMargins(5, 5, 5, 5));
        _statsText->setSelectable(false);
        _statsText->setLayer("overlay");
        _statsText->setVisible(false);
    }
}

void StatsManager::destroyTextItem()
{
    if (_statsText && _plot) {
        _plot->removeItem(_statsText);
        _statsText = nullptr;
    }
}

void StatsManager::updateStats()
{
    if (!_visible || !_plot || !_statsText)
        return;

    QLocale locale;
    QString statsText;
    for (int i = 0; i < _plot->graphCount(); ++i) {
        QCPGraph *graph = _plot->graph(i);
        if (!graph || !graph->visible())
            continue;

        GraphStats stats = computeStatsForGraph(graph);
        if (stats.pointCount == 0)
            continue;

#define STATS_HEADER_TEMPLATE   QString("%1 (n=%2)\n")
#define STATS_TEMPLATE          QString("min: %1, max: %2\n mean: %3, σ: %4\n")

        statsText += STATS_HEADER_TEMPLATE
                         .arg(stats.name)
                         .arg(stats.pointCount);
        statsText += STATS_TEMPLATE
                         .arg(locale.toString(stats.minValue, 'f', 3))
                         .arg(locale.toString(stats.maxValue, 'f', 3))
                         .arg(locale.toString(stats.mean, 'f', 3))
                         .arg(locale.toString(stats.stdDev, 'f', 3));
    }

    if (statsText.isEmpty()) {
        _statsText->setText("Нет данных");
    } else {
        _statsText->setText(statsText.trimmed());
    }
    _plot->replot(QCustomPlot::rpQueuedReplot);
}

StatsManager::GraphStats StatsManager::computeStatsForGraph(QCPGraph *graph) const
{
    GraphStats stats;
    if (!graph)
        return stats;

    stats.name = graph->name();
    auto data = graph->data();
    if (!data || data->isEmpty())
        return stats;

    double minVal = std::numeric_limits<double>::max();
    double maxVal = -std::numeric_limits<double>::max();
    double sum = 0.0;
    double sumSq = 0.0;
    int count = 0;

    if (_onlyVisibleRange) {
        QCPAxis *keyAxis = graph->keyAxis();
        QCPAxis *valueAxis = graph->valueAxis();
        if (!keyAxis || !valueAxis)
            return stats;

        double keyLow = keyAxis->range().lower;
        double keyHigh = keyAxis->range().upper;
        double valLow = valueAxis->range().lower;
        double valHigh = valueAxis->range().upper;

        // Итерация по всем точкам (можно оптимизировать, если точек много)
        for (auto it = data->begin(); it != data->end(); ++it) {
            if (it->key >= keyLow && it->key <= keyHigh &&
                it->value >= valLow && it->value <= valHigh) {
                double v = it->value;
                minVal = qMin(minVal, v);
                maxVal = qMax(maxVal, v);
                sum += v;
                sumSq += v * v;
                ++count;
            }
        }
    } else {
        // все точки
        for (auto it = data->begin(); it != data->end(); ++it) {
            double v = it->value;
            minVal = qMin(minVal, v);
            maxVal = qMax(maxVal, v);
            sum += v;
            sumSq += v * v;
            ++count;
        }
    }

    if (count == 0)
        return stats;

    stats.pointCount = count;
    stats.minValue = minVal;
    stats.maxValue = maxVal;
    stats.mean = sum / count;
    double variance = (sumSq / count) - (stats.mean * stats.mean);
    stats.stdDev = std::sqrt(variance);

    return stats;
}
