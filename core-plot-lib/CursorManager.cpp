#include "CursorManager.h"

#include <QObject>

#include "CustomPlotBase.h"

CursorManager::CursorManager(CustomPlotBase *plot)
    : _plot(plot)
{
}

CursorManager::~CursorManager()
{
    destroyItems();
}

void CursorManager::setMode(Mode mode)
{
    if (_mode == mode)
        return;
    _mode = mode;
    if (_mode == Mode::NoCursor) {
        destroyItems();
    } else {
        createItems();
        updateVisibility();
        // Попробуем сразу получить текущую позицию мыши
        QPoint cursorPos = QCursor::pos();
        if (_plot) {
            QPoint localPos = _plot->mapFromGlobal(cursorPos);
            if (_plot->rect().contains(localPos))
                handleMouseMove(localPos);
        }
    }
    _plot->replot();
}

void CursorManager::handleMouseMove(const QPoint &pixelPos)
{
    if (_mode == Mode::NoCursor || !_plot)
        return;

    ClosestPointInfo pointInfo = findClosestPoint(pixelPos, _mode);
    if (!pointInfo.isValid())
        return;

    QPointF plotPos(pointInfo.key, pointInfo.value);
    updatePosition(plotPos, pointInfo);
    _plot->replot(QCustomPlot::rpQueuedReplot);
}

void CursorManager::handleAxisRangeChanged()
{
    if (_mode == Mode::NoCursor || !_plot)
        return;
    QPoint cursorPos = _plot->mapFromGlobal(QCursor::pos());
    if (_plot->rect().contains(cursorPos))
        handleMouseMove(cursorPos);
}

void CursorManager::createItems()
{
    if (!_plot)
        return;

    if (!_verticalLine) {
        _verticalLine = new QCPItemLine(_plot);
        _verticalLine->setPen(QPen(Qt::black, 1, Qt::DashLine));
        _verticalLine->setSelectable(false);
        _verticalLine->setLayer("overlay");
    }
    if (!_horizontalLine) {
        _horizontalLine = new QCPItemLine(_plot);
        _horizontalLine->setPen(QPen(Qt::black, 1, Qt::DashLine));
        _horizontalLine->setSelectable(false);
        _horizontalLine->setLayer("overlay");
    }
    if (!_cursorText) {
        _cursorText = new QCPItemText(_plot);
        _cursorText->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
        _cursorText->position->setType(QCPItemPosition::ptPlotCoords);
        _cursorText->setFont(QFont("sans", 8));
        _cursorText->setColor(Qt::black);
        _cursorText->setBrush(QBrush(Qt::white));
        _cursorText->setPadding(QMargins(2, 2, 2, 2));
        _cursorText->setSelectable(false);
        _cursorText->setLayer("overlay");
    }
}

void CursorManager::destroyItems()
{
    if (!_plot)
        return;

    if (_verticalLine) {
        _plot->removeItem(_verticalLine);
        _verticalLine = nullptr;
    }
    if (_horizontalLine) {
        _plot->removeItem(_horizontalLine);
        _horizontalLine = nullptr;
    }
    if (_cursorText) {
        _plot->removeItem(_cursorText);
        _cursorText = nullptr;
    }
}

QString CursorManager::formatKey(double key, QCPAxis *axis) const
{
    QLocale locale;

    if (!axis)
        return locale.toString(key, 'f', 3);

    QSharedPointer<QCPAxisTicker> ticker = axis->ticker();
    if (!ticker)
        return locale.toString(key, 'f', 3);

    // Способ 1: dynamic_cast (требует RTTI, обычно включено по умолчанию)
    QCPAxisTickerDateTime *dtTicker = dynamic_cast<QCPAxisTickerDateTime*>(ticker.data());
    if (dtTicker) {
        QDateTime dt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(key));
        QString format = dtTicker->dateTimeFormat();
        if (format.isEmpty())
            format = DATE_TIME_FORMAT_WITHOUT_MS;
        return dt.toString(format);
    }

    // Способ 2: если dynamic_cast не работает, используйте typeid
    // if (typeid(*ticker) == typeid(QCPAxisTickerDateTime)) {
    //     auto *dtTicker = static_cast<QCPAxisTickerDateTime*>(ticker.data());
    //     // ...
    // }

    return locale.toString(key, 'f', 3);
}

void CursorManager::updateVisibility()
{
    if (!_verticalLine || !_horizontalLine || !_cursorText)
        return;

    switch (_mode) {
    case Mode::NoCursor:
        _verticalLine->setVisible(false);
        _horizontalLine->setVisible(false);
        _cursorText->setVisible(false);
        break;
    case Mode::VerticalCursor:
        _verticalLine->setVisible(true);
        _horizontalLine->setVisible(false);
        _cursorText->setVisible(true);
        break;
    case Mode::HorizontalCursor:
        _verticalLine->setVisible(false);
        _horizontalLine->setVisible(true);
        _cursorText->setVisible(true);
        break;
    case Mode::CrossCursor:
        _verticalLine->setVisible(true);
        _horizontalLine->setVisible(true);
        _cursorText->setVisible(true);
        break;
    }
}

void CursorManager::updatePosition(const QPointF &plotPos, const ClosestPointInfo &pointInfo)
{
    if (!_verticalLine || !_horizontalLine || !_cursorText || !pointInfo.isValid())
        return;

    double x = plotPos.x();
    double y = plotPos.y();

    QCPAxis *keyAxis = pointInfo.graph->keyAxis();
    QCPAxis *valueAxis = pointInfo.graph->valueAxis();
    if (!keyAxis || !valueAxis)
        return;

    // Устанавливаем оси для линий и текста, чтобы они были в системе координат графика
    if (_verticalLine) {
        _verticalLine->start->setAxes(keyAxis, valueAxis);
        _verticalLine->end->setAxes(keyAxis, valueAxis);
        _verticalLine->start->setCoords(x, valueAxis->range().lower);
        _verticalLine->end->setCoords(x, valueAxis->range().upper);
    }
    if (_horizontalLine) {
        _horizontalLine->start->setAxes(keyAxis, valueAxis);
        _horizontalLine->end->setAxes(keyAxis, valueAxis);
        _horizontalLine->start->setCoords(keyAxis->range().lower, y);
        _horizontalLine->end->setCoords(keyAxis->range().upper, y);
    }

    _cursorText->position->setAxes(keyAxis, valueAxis);
    _cursorText->position->setCoords(x, y);

    // Форматируем X как дату/время, если ось использует datetime тикер
    QString xStr = formatKey(x, keyAxis);
    QLocale locale;
    QString yStr = locale.toString(y, 'f', 3);  // Y можно тоже форматировать по необходимости

    QString text = QString("x=%1\ny=%2")
                       .arg(xStr)
                       .arg(yStr);
    _cursorText->setText(text);
}

ClosestPointInfo CursorManager::findClosestPoint(const QPoint &pixelPos, Mode mode) const
{
    if (!_plot)
        return ClosestPointInfo();

    ClosestPointInfo best;
    best.distance = std::numeric_limits<double>::max();

    for (int i = 0; i < _plot->graphCount(); ++i) {
        QCPGraph *graph = _plot->graph(i);
        if (!graph || !graph->visible())
            continue;

        auto data = graph->data(); // QSharedPointer<QCPDataContainer<QCPGraphData>>
        if (!data || data->isEmpty())
            continue;

        QCPAxis *keyAxis = graph->keyAxis();
        QCPAxis *valueAxis = graph->valueAxis();
        if (!keyAxis || !valueAxis)
            continue;

        double keyCoord = keyAxis->pixelToCoord(pixelPos.x());
        double valueCoord = valueAxis->pixelToCoord(pixelPos.y());

        if (mode == Mode::VerticalCursor) {
            auto it = data->findBegin(keyCoord);
            if (it != data->end()) {
                double dist = std::abs(it->key - keyCoord);
                if (dist < best.distance) {
                    best.graph = graph;
                    best.key = it->key;
                    best.value = it->value;
                    best.distance = dist;
                }
            }
            if (it != data->begin()) {
                auto prev = it;
                --prev;
                double dist = std::abs(prev->key - keyCoord);
                if (dist < best.distance) {
                    best.graph = graph;
                    best.key = prev->key;
                    best.value = prev->value;
                    best.distance = dist;
                }
            }
        }
        else if (mode == Mode::HorizontalCursor) {
            for (auto it = data->begin(); it != data->end(); ++it) {
                double dist = std::abs(it->value - valueCoord);
                if (dist < best.distance) {
                    best.graph = graph;
                    best.key = it->key;
                    best.value = it->value;
                    best.distance = dist;
                }
            }
        }
        else if (mode == Mode::CrossCursor) {
            for (auto it = data->begin(); it != data->end(); ++it) {
                double dx = it->key - keyCoord;
                double dy = it->value - valueCoord;
                double dist = dx*dx + dy*dy;
                if (dist < best.distance) {
                    best.graph = graph;
                    best.key = it->key;
                    best.value = it->value;
                    best.distance = dist;
                }
            }
        }
    }

    return best;
}
