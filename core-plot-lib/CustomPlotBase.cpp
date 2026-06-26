#include "CustomPlotBase.h"

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPointer>

// Сохранение ThresholdLine
void ThresholdLine::saveSettings(QSettings& settings, int index) const {
    settings.setValue(QString("value_%1").arg(index), value);
    settings.setValue(QString("color_%1").arg(index), color);
    settings.setValue(QString("style_%1").arg(index), static_cast<int>(style));
    settings.setValue(QString("width_%1").arg(index), width);
    settings.setValue(QString("text_%1").arg(index), text);
    settings.setValue(QString("axisLabel_%1").arg(index), axis->label());
}

bool ThresholdLine::loadSettings(QSettings& settings, int index, const QList<QCPAxis*>& availableAxes) {
    value = settings.value(QString("value_%1").arg(index)).toDouble();
    color = settings.value(QString("color_%1").arg(index)).value<QColor>();
    style = static_cast<Qt::PenStyle>(settings.value(QString("style_%1").arg(index), static_cast<int>(Qt::DashLine)).toInt());
    width = settings.value(QString("width_%1").arg(index), 1).toInt();
    text = settings.value(QString("text_%1").arg(index)).toString();
    QString axisLabel = settings.value(QString("axisLabel_%1").arg(index)).toString();

    // Ищем ось с такой меткой
    axis = nullptr;
    for (QCPAxis* ax : availableAxes) {
        if (ax->label() == axisLabel) {
            axis = ax;
            break;
        }
    }
    return axis != nullptr;
}

void VerticalCursor::saveSettings(QSettings& settings, int index) const {
    settings.setValue(QString("xValue_%1").arg(index), xValue);
    settings.setValue(QString("color_%1").arg(index), color);
    settings.setValue(QString("width_%1").arg(index), width);
    settings.setValue(QString("style_%1").arg(index), static_cast<int>(style));
    settings.setValue(QString("visible_%1").arg(index), visible);
}

bool VerticalCursor::loadSettings(QSettings& settings, int index) {
    xValue = settings.value(QString("xValue_%1").arg(index)).toDouble();
    color  = settings.value(QString("color_%1").arg(index), QColor(Qt::blue)).value<QColor>();
    width  = settings.value(QString("width_%1").arg(index), 1).toInt();
    style  = static_cast<Qt::PenStyle>(settings.value(QString("style_%1").arg(index), static_cast<int>(Qt::DashDotLine)).toInt());
    visible = settings.value(QString("visible_%1").arg(index), true).toBool();
    labelText = QDateTime::fromSecsSinceEpoch(xValue).toString(DATE_TIME_FORMAT_WITHOUT_MS);
    return true;
}

CustomPlotBase::CustomPlotBase(const QString &objectName, QWidget *parent) :
    QCustomPlot(parent)
{
    this->setObjectName(objectName);

    this->setWindowFlags(Qt::Window);

    this->setMinimumSize(QSize(200, 200));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->setSizePolicy(sizePolicy);
    this->setPlottingHint(QCP::PlottingHint::phCacheLabels, false);

    //this->axisRect()->setMargins(QMargins(100, 100, 100, 100));

    if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
        this->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(75); //must be for axis tag
    }

    setSelectionRectMode(QCP::srmNone);

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
    {
        connect(xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, &CustomPlotBase::updateThresholdLines);
    }

    this->loadAllSettings();
}

CustomPlotBase::~CustomPlotBase() {
    saveAllSettings(); //before reset managers
    _statsManager.reset();
    _cursorManager.reset();
}

void CustomPlotBase::export_data(QTextStream &stream) {
    bool isDateTime = dynamic_cast<QCPAxisTickerDateTime*>(xAxis->ticker().data()) != nullptr;
    QLocale locale;

    // Заголовки: имя графика, единица измерения, ось X, значение
    QStringList headers = { tr("График"), tr("Единица"), xAxis->label(), tr("Значение") };
    stream << headers.join(ELEMENTS_DELIMETER) << ROW_DELIMETER;

    for (int i = 0; i < graphCount(); ++i) {
        QCPGraph *graph = this->graph(i);
        if (!graph->visible()) continue;          // только видимые

        QString name = graph->name();
        QString unit = graph->valueAxis()->label();  // берём подпись оси Y (например, "мг/л")

        for (const auto &d : *graph->data()) {
            QStringList row;
            row << name << unit;

            if (isDateTime)
                row << QDateTime::fromSecsSinceEpoch(d.key).toString(DATE_TIME_FORMAT_WITHOUT_MS);
            else
                row << locale.toString(d.key);

            row << locale.toString(d.value);
            stream << row.join(ELEMENTS_DELIMETER) << ROW_DELIMETER;
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

void CustomPlotBase::saveAllSettings() {
    QSettings settings;
    settings.beginGroup(this->objectName());

    // 1. Диапазон оси X
    settings.setValue("xAxis_lower", xAxis->range().lower);
    settings.setValue("xAxis_upper", xAxis->range().upper);

    // 2. Диапазон левой оси Y
    settings.setValue("yAxis_lower", yAxis->range().lower);
    settings.setValue("yAxis_upper", yAxis->range().upper);

    // 3. Диапазоны правых осей (ключ – метка оси, она же единица измерения)
    for (QCPAxis* ax : _rightAxes) {
        if (!ax->visible()) continue; // сохраняем только видимые
        QString key = QString("rightAxis_%1").arg(ax->label());
        settings.setValue(key + "_lower", ax->range().lower);
        settings.setValue(key + "_upper", ax->range().upper);
    }

    // 4. Состояние автомасштаба и легенды
    settings.setValue("autoScale", _autorescale);
    settings.setValue("legendVisible", _legend_state);

    // 5. Позиция легенды
    if (axisRect()->insetLayout()->elementCount() > 0) {
        Qt::Alignment align = axisRect()->insetLayout()->insetAlignment(0);
        settings.setValue("legendAlignment", (int)align);
    }

    // 6. Режим курсора
    settings.setValue("cursorMode", (int)_cursorManager->mode());

    // 7. Статистика
    settings.setValue("statsVisible", _statsManager->isVisible());
    settings.setValue("statsOnlyVisibleRange", _statsManager->showOnlyVisibleRange());

    // 8. Пороговые линии
    saveThresholdLines();

    // 9. Вертикальные курсоры
    saveVerticalCursors();

    // Стили графиков сохраняются отдельно при изменении, здесь не дублируем.
    settings.endGroup();
}

void CustomPlotBase::loadAllSettings() {
    QSettings settings;
    settings.beginGroup(this->objectName());

    // 1. Автомасштаб и легенда
    _autorescale = settings.value("autoScale", true).toBool();
    _legend_state = settings.value("legendVisible", true).toBool();
    legend->setVisible(_legend_state);
    if (axisRect()->insetLayout()->elementCount() > 0) {
        Qt::Alignment align = (Qt::Alignment)settings.value("legendAlignment",
                                                             (int)(Qt::AlignBottom | Qt::AlignRight)).toInt();
        axisRect()->insetLayout()->setInsetAlignment(0, align);
    }

    // 2. Диапазон оси X
    if (!_autorescale) {
        double xLower = settings.value("xAxis_lower", xAxis->range().lower).toDouble();
        double xUpper = settings.value("xAxis_upper", xAxis->range().upper).toDouble();
        if (xLower < xUpper)
            xAxis->setRange(xLower, xUpper);
    }

    // 3. Левая ось Y
    if (!_autorescale) {
        double yLower = settings.value("yAxis_lower", yAxis->range().lower).toDouble();
        double yUpper = settings.value("yAxis_upper", yAxis->range().upper).toDouble();
        if (yLower < yUpper)
            yAxis->setRange(yLower, yUpper);
    }

    // 4. Правые оси (восстанавливаем, только если автомасштаб выключен)
    if (!_autorescale) {
        for (QCPAxis* ax : _rightAxes) {
            if (!ax->visible()) continue;
            QString key = QString("rightAxis_%1").arg(ax->label());
            double lower = settings.value(key + "_lower", ax->range().lower).toDouble();
            double upper = settings.value(key + "_upper", ax->range().upper).toDouble();
            if (lower < upper)
                ax->setRange(lower, upper);
        }
    }

    // 5. Режим курсора
    _cursorManager->setMode((CursorManager::Mode)settings.value("cursorMode",
                                                                 (int)CursorManager::Mode::NoCursor).toInt());

    // 6. Статистика
    _statsManager->setVisible(settings.value("statsVisible", false).toBool());
    _statsManager->setShowOnlyVisibleRange(settings.value("statsOnlyVisibleRange", true).toBool());

    // 7. Пороговые линии
    QList<QCPAxis*> availableAxes;
    availableAxes << yAxis;
    for (QCPAxis* ax : _rightAxes) availableAxes << ax;
    loadThresholdLines(availableAxes);

    // 8. Вертикальные курсоры
    loadVerticalCursors();

    // Стили графиков будут применены при создании каждого графика через applySavedStyle().
    settings.endGroup();

    replot();
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

void CustomPlotBase::addThresholdLine(QCPAxis* axis, double value,
                                      const QColor& color, Qt::PenStyle style,
                                      int width, const QString& text)
{
    ThresholdLine tl;
    tl.axis = axis;
    tl.value = value;
    tl.color = color;
    tl.style = style;
    tl.width = width;
    tl.text = text;

    // Создаём линию
    auto* line = new QCPItemStraightLine(this);
    line->setPen(QPen(color, width, style));
    line->point1->setCoords(xAxis->range().lower, value);
    line->point2->setCoords(xAxis->range().upper, value);
    line->point1->setAxes(xAxis, axis);
    line->point2->setAxes(xAxis, axis);
    line->setClipToAxisRect(true);
    tl.line = line;

    // Метка (если задан текст)
    if (!text.isEmpty()) {
        auto* textItem = new QCPItemText(this);
        textItem->setText(text);
        QCPRange yRange = axis->range();
        double offset = (yRange.upper - yRange.lower) * 0.02;
        if (qFuzzyIsNull(offset)) offset = 1.0;   // страховка при нулевом диапазоне
        textItem->position->setCoords(xAxis->range().upper, value + offset);
        textItem->position->setAxes(xAxis, axis);
        textItem->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
        textItem->setPen(QPen(color));
        textItem->setBrush(Qt::NoBrush);
        textItem->setClipToAxisRect(false);
        tl.label = textItem;
    }

    _thresholdLines.append(tl);
    replot();
    saveThresholdLines(); // сохраняем немедленно
}

void CustomPlotBase::removeThresholdLine(int index) {
    if (index < 0 || index >= _thresholdLines.size()) return;
    ThresholdLine& tl = _thresholdLines[index];
    if (tl.line) {
        removeItem(tl.line); // QCustomPlot::removeItem
        // removeItem удаляет элемент и освобождает память
    }
    if (tl.label) removeItem(tl.label);
    _thresholdLines.removeAt(index);
    replot();
    saveThresholdLines();
}

void CustomPlotBase::clearThresholdLines() {
    while (!_thresholdLines.isEmpty())
        removeThresholdLine(0);
}

void CustomPlotBase::updateThresholdLines() {
    QCPRange xRange = xAxis->range();
    for (ThresholdLine& tl : _thresholdLines) {
        if (!tl.line) continue;
        tl.line->point1->setCoords(xRange.lower, tl.value);
        tl.line->point2->setCoords(xRange.upper, tl.value);
        if (tl.label) {
            QCPRange yRange = tl.axis->range();
            double offset = (yRange.upper - yRange.lower) * 0.02;
            if (qFuzzyIsNull(offset)) offset = 1.0;
            tl.label->position->setCoords(xRange.upper, tl.value + offset);
        }
    }
    replot(QCustomPlot::rpQueuedReplot);
}

void CustomPlotBase::saveThresholdLines() {
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.beginGroup("ThresholdLines");
    settings.beginWriteArray("lines");
    for (int i = 0; i < _thresholdLines.size(); ++i) {
        settings.setArrayIndex(i);
        _thresholdLines[i].saveSettings(settings, i);
    }
    settings.endArray();
    settings.endGroup();
    settings.endGroup();
}

void CustomPlotBase::loadThresholdLines(const QList<QCPAxis*>& availableAxes) {
    clearThresholdLines();
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.beginGroup("ThresholdLines");
    int size = settings.beginReadArray("lines");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ThresholdLine tl;
        if (tl.loadSettings(settings, i, availableAxes)) {
            // Создаём графические элементы
            auto* line = new QCPItemStraightLine(this);
            line->setPen(QPen(tl.color, tl.width, tl.style));
            line->point1->setCoords(xAxis->range().lower, tl.value);
            line->point2->setCoords(xAxis->range().upper, tl.value);
            line->point1->setAxes(xAxis, tl.axis);
            line->point2->setAxes(xAxis, tl.axis);
            line->setClipToAxisRect(true);
            tl.line = line;

            if (!tl.text.isEmpty()) {
                auto* textItem = new QCPItemText(this);
                textItem->setText(tl.text);
                textItem->position->setCoords(xAxis->range().upper, tl.value);
                textItem->position->setAxes(xAxis, tl.axis);
                textItem->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
                textItem->setPen(QPen(tl.color));
                textItem->setBrush(Qt::NoBrush);
                textItem->setClipToAxisRect(false);
                tl.label = textItem;
            }

            _thresholdLines.append(tl);
        }
    }
    settings.endArray();
    settings.endGroup();
    settings.endGroup();
    updateThresholdLines(); // установить правильные координаты
}

void CustomPlotBase::addVerticalCursor(double xValue, const QColor& color, int width) {
    VerticalCursor vc;
    vc.xValue = xValue;
    vc.color = color;
    vc.width = width;
    vc.visible = true;
    vc.labelText = QDateTime::fromSecsSinceEpoch(xValue).toString(DATE_TIME_FORMAT_WITHOUT_MS);

    // Линия
    auto* line = new QCPItemStraightLine(this);
    line->setPen(QPen(color, width, Qt::DashDotLine));
    line->point1->setCoords(xValue, yAxis->range().lower);
    line->point2->setCoords(xValue, yAxis->range().upper);
    line->point1->setAxes(xAxis, yAxis);
    line->point2->setAxes(xAxis, yAxis);
    line->setClipToAxisRect(true);
    vc.line = line;

    // Метка вверху
    auto* text = new QCPItemText(this);
    text->setText(vc.labelText);
    text->position->setCoords(xValue, yAxis->range().upper);
    text->position->setAxes(xAxis, yAxis);
    text->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    text->setPen(QPen(color));
    text->setBrush(Qt::NoBrush);
    text->setClipToAxisRect(false);
    vc.label = text;

    _verticalCursors.append(vc);
    replot();
    saveVerticalCursors();
}

void CustomPlotBase::removeVerticalCursor(int index) {
    if (index < 0 || index >= _verticalCursors.size()) return;
    VerticalCursor& vc = _verticalCursors[index];
    if (vc.line)  removeItem(vc.line);
    if (vc.label) removeItem(vc.label);
    _verticalCursors.removeAt(index);
    replot();
    saveVerticalCursors();
}

void CustomPlotBase::clearVerticalCursors() {
    while (!_verticalCursors.isEmpty())
        removeVerticalCursor(0);
}

void CustomPlotBase::updateCursorLabels() {
    // Обновляет метки после изменения масштаба оси Y
    QCPRange yRange = yAxis->range();
    for (auto& vc : _verticalCursors) {
        if (vc.line) {
            vc.line->point1->setCoords(vc.xValue, yRange.lower);
            vc.line->point2->setCoords(vc.xValue, yRange.upper);
        }
        if (vc.label) {
            vc.label->position->setCoords(vc.xValue, yRange.upper);
        }
    }
}

void CustomPlotBase::saveVerticalCursors() {
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.beginGroup("VerticalCursors");
    settings.beginWriteArray("cursors");
    for (int i = 0; i < _verticalCursors.size(); ++i) {
        settings.setArrayIndex(i);
        _verticalCursors[i].saveSettings(settings, i);
    }
    settings.endArray();
    settings.endGroup();
    settings.endGroup();
}

void CustomPlotBase::loadVerticalCursors() {
    clearVerticalCursors();
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.beginGroup("VerticalCursors");
    int size = settings.beginReadArray("cursors");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        VerticalCursor vc;
        if (vc.loadSettings(settings, i)) {
            // Создаём графические элементы
            auto* line = new QCPItemStraightLine(this);
            line->setPen(QPen(vc.color, vc.width, vc.style));
            line->point1->setCoords(vc.xValue, yAxis->range().lower);
            line->point2->setCoords(vc.xValue, yAxis->range().upper);
            line->point1->setAxes(xAxis, yAxis);
            line->point2->setAxes(xAxis, yAxis);
            line->setClipToAxisRect(true);
            vc.line = line;

            auto* text = new QCPItemText(this);
            text->setText(vc.labelText);
            text->position->setCoords(vc.xValue, yAxis->range().upper);
            text->position->setAxes(xAxis, yAxis);
            text->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
            text->setPen(QPen(vc.color));
            text->setBrush(Qt::NoBrush);
            text->setClipToAxisRect(false);
            vc.label = text;

            _verticalCursors.append(vc);
        }
    }
    settings.endArray();
    settings.endGroup();
    settings.endGroup();
}

void CustomPlotBase::move_legend(int pos) {
    this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)pos);
    this->replot();
}

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
            this->set_scale_home();
            return;
            break;
        default:

            break;
    }

    QCustomPlot::mousePressEvent(event);
}

void CustomPlotBase::mouseDoubleClickEvent(QMouseEvent *event) {
    // Если клик пришёлся по легенде
    if (legend->selectTest(event->pos(), false) >= 0) {
        // Перебираем все элементы легенды и проверяем, в какой попала точка
        for (int i = 0; i < legend->itemCount(); ++i) {
            QCPAbstractLegendItem *item = legend->item(i);
            if (item && item->rect().contains(event->pos())) {
                QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
                if (plItem) {
                    QCPAbstractPlottable *plottable = plItem->plottable();
                    QCPGraph *graph = qobject_cast<QCPGraph*>(plottable);
                    if (graph) {
                        showStyleDialog(graph);   // открываем диалог настройки стиля
                        return;
                    }
                }
            }
        }
    }
    // Иначе стандартная обработка
    QCustomPlot::mouseDoubleClickEvent(event);
}

void CustomPlotBase::set_scale_home() {
    // this->yAxis->rescale();
    // this->yAxis2->rescale();
    // this->xAxis->rescale();
    // for (QCPAxis *ax : _rightAxes) {
    //     ax->rescale();
    // }
    // // if (this->axisRect()->axis(QCPAxis::atRight, 1)) {
    // //     this->axisRect()->axis(QCPAxis::atRight, 1)->rescale();
    // // }
    // this->replot();
    xAxis->rescale();
    this->yAxis2->rescale();
    yAxis->rescale();
    for (QCPAxis *ax : _rightAxes) ax->rescale();
    updateThresholdLines();
    updateCursorLabels();
    this->replot();
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
    QString dir = "График " + QDateTime::currentDateTime().toString(DATE_TIME_FORMAT_WITHOUT_MS) + ".csv"; //cfg.TEST_RESULT_DIR +
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

        QMessageBox::information(this, "Экспорт", QString("График успешно экспортирован в файл:\n%1").arg(filename));
    }
}

void CustomPlotBase::export_png() {
    QString dir = "График " + QDateTime::currentDateTime().toString(DATE_TIME_FORMAT_WITHOUT_MS) + ".png"; //cfg.TEST_RESULT_DIR +
    QString filename = QFileDialog::getSaveFileName(this,
            tr("%1 - Сохранить").arg(QApplication::applicationDisplayName()),
            dir,
            tr("(*.png)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);

    if (filename != "") {
        this->savePng(filename);

        QMessageBox::information(this, "Экспорт", QString("График успешно экспортирован в файл:\n%1").arg(filename));
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
    } else {
        addScaleActions(menu);

        // Специфические для наследников пункты
        addGraphSpecificActions(menu);

        addThresholdLinesActions(menu);
        //addVerticalCursorActions(menu);

        if (selectedGraphs().size() == 1) {
            QCPGraph *selected = selectedGraphs().first();
            menu->addAction(tr("Настроить стиль графика..."), this, [this, selected]() {
                showStyleDialog(selected);
            });
        }

        // Показать/скрыть легенду
        if (get_legend_state()) {
            menu->addAction(tr("Скрыть легенду"), [this](){ set_legend_state(false); });
        } else {
            menu->addAction(tr("Показать легенду"), [this](){ set_legend_state(true); });
        }

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

QColor CustomPlotBase::defaultGraphColor(int index) const {
    return (index < PEN_COLOR_EXT_v.size()) ? QColor(PEN_COLOR_EXT_v[index])
                                        : QColor::fromHsv(index * 50 % 360, 255, 200);
}

void CustomPlotBase::applyGraphStyle(QCPGraph* graph, const GraphStyle& style) {
    if (!graph) return;
    QPen pen(style.color, style.width, style.penStyle);
    graph->setPen(pen);
    graph->selectionDecorator()->setPen(QPen(style.color.darker(), style.width));
    replot(QCustomPlot::rpQueuedReplot);
}

void CustomPlotBase::applySavedStyle(QCPGraph* graph, const QString& styleKey) {
    if (!graph) return;
    int idx = graph->property("colorIndex").toInt();
    GraphStyle style = loadStyle(styleKey, idx);
    applyGraphStyle(graph, style);
}

void CustomPlotBase::saveStyle(const QString& styleKey, const GraphStyle& style) {
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.beginGroup("GraphStyles");
    settings.setValue(styleKey + "_color", style.color);
    settings.setValue(styleKey + "_width", style.width);
    settings.setValue(styleKey + "_lineStyle", (int)style.lineStyle);
    settings.setValue(styleKey + "_penStyle", (int)style.penStyle);
    settings.endGroup();
    settings.endGroup();
}

GraphStyle CustomPlotBase::loadStyle(const QString& styleKey, int colorIndex) const {
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.beginGroup("GraphStyles");
    GraphStyle style;
    style.color = settings.value(styleKey + "_color", defaultGraphColor(colorIndex)).value<QColor>();
    style.width = settings.value(styleKey + "_width", defaultGraphWidth()).toInt();
    style.lineStyle = static_cast<QCPGraph::LineStyle>(
        settings.value(styleKey + "_lineStyle", (int)defaultGraphLineStyle()).toInt()
        );
    style.penStyle = static_cast<Qt::PenStyle>(
        settings.value(styleKey + "_penStyle", (int)defaultGraphPenStyle()).toInt()
        );
    settings.endGroup();
    settings.endGroup();
    return style;
}

void CustomPlotBase::showStyleDialog(QCPGraph* graph) {
    if (!graph) return;
    QString key = graph->property("styleKey").toString();
    if (key.isEmpty()) return;

    int colorIdx = graph->property("colorIndex").toInt();
    GraphStyle style = loadStyle(key, colorIdx);

    // 1. Цвет
    QColor newColor = QColorDialog::getColor(style.color, this, tr("Цвет графика"));
    if (!newColor.isValid()) return;

    // 2. Толщина
    bool ok;
    int newWidth = QInputDialog::getInt(this, tr("Толщина линии"),
                                        tr("Толщина (1–10):"), style.width, 1, 10, 1, &ok);
    if (!ok) return;

    // 3. Стиль линии
    QStringList styleNames = {
        tr("Сплошная"),
        tr("Пунктир"),
        tr("Тире"),
        tr("Штрих-пунктир"),
        tr("Штрих-точка-точка")
    };
    QVector<Qt::PenStyle> penStyles = {
        Qt::PenStyle::SolidLine,
        Qt::PenStyle::DashLine,
        Qt::PenStyle::DotLine,
        Qt::PenStyle::DashDotLine,
        Qt::PenStyle::DashDotDotLine
    };
    int currentIdx = penStyles.indexOf(style.penStyle);
    if (currentIdx < 0) currentIdx = 0;
    QString selected = QInputDialog::getItem(this, tr("Стиль линии"),
                                             tr("Стиль:"), styleNames, currentIdx, false, &ok);
    if (!ok) return;
    int idx = styleNames.indexOf(selected);
    if (idx < 0) idx = 0;

    GraphStyle newStyle { newColor, newWidth, this->defaultGraphLineStyle(), penStyles[idx] };
    applyGraphStyle(graph, newStyle);
    saveStyle(key, newStyle);
}

void CustomPlotBase::addScaleActions(QMenu *menu) {
    //Масштаб
    QMenu *scaleMenu = menu->addMenu(tr("Масштаб"));
    // Автомасштаб
    if (get_autorescale()) {
        scaleMenu->addAction(tr("Выключить автомасштаб"), [this](){ set_autorescale(false); });
    } else {
        scaleMenu->addAction(tr("Включить автомасштаб"), [this](){ set_autorescale(true); });
    }

    QAction *resetScale = scaleMenu->addAction(tr("Сбросить масштаб"));
    connect(resetScale, &QAction::triggered, this, &CustomPlotBase::set_scale_home);

    QAction *zoomAction = scaleMenu->addAction(tr("Режим \"Лупа\""));
    zoomAction->setCheckable(true);
    zoomAction->setChecked(_zoomRectMode);
    connect(zoomAction, &QAction::toggled, this, [this](bool enabled) {
        _zoomRectMode = enabled;
        if (enabled) {
            setSelectionRectMode(QCP::srmZoom);
            setCursor(Qt::CrossCursor);
        } else {
            setSelectionRectMode(QCP::srmNone);
            setCursor(Qt::ArrowCursor);
        }
    });

    addAxisActions(scaleMenu);
}

void CustomPlotBase::addAxisActions(QMenu *menu) {
    // Сбор всех видимых осей
    QList<QCPAxis*> visibleAxes;
    visibleAxes << xAxis << yAxis;
    for (QCPAxis* ax : _rightAxes) {
        if (ax->visible())
            visibleAxes << ax;
    }

    if (!visibleAxes.isEmpty()) {
        QMenu* axisMenu = menu->addMenu(tr("Оси"));
        for (QCPAxis* ax : visibleAxes) {
            QString label;
            if (ax == xAxis)
                label = tr("Ось X (%1)").arg(ax->label());
            else if (ax == yAxis)
                label = tr("Ось Y (%1)").arg(ax->label());
            else
                label = tr("Правая ось (%1)").arg(ax->label());
            axisMenu->addAction(label, this, [this, ax]() {
                showAxisRangeDialog(ax);
            });
        }
    }
}

void CustomPlotBase::addThresholdLinesActions(QMenu *menu) {
    QMenu* thresholdMenu = menu->addMenu(tr("Пороги"));
    thresholdMenu->addAction(tr("Добавить пороговую линию..."), this, &CustomPlotBase::showAddThresholdDialog);
    if (!_thresholdLines.isEmpty()) {
        thresholdMenu->addSeparator();
        for (int i = 0; i < _thresholdLines.size(); ++i) {
            QString desc = _thresholdLines[i].text.isEmpty()
            ? QString("Y = %1").arg(_thresholdLines[i].value)
            : _thresholdLines[i].text;
            thresholdMenu->addAction(tr("Редактировать %1...").arg(desc),
                                     this, [this, i]() { showEditThresholdDialog(i); });
            thresholdMenu->addAction(tr("Удалить %1").arg(desc),
                                     this, [this, i]() { removeThresholdLine(i); });
        }
    }
}

void CustomPlotBase::addVerticalCursorActions(QMenu* menu) {
    QMenu* cursorMenu = menu->addMenu(tr("Вертикальные курсоры"));
    cursorMenu->addAction(tr("Добавить курсор..."), this, &CustomPlotBase::showAddVerticalCursorDialog);
    if (!_verticalCursors.isEmpty()) {
        cursorMenu->addSeparator();
        for (int i = 0; i < _verticalCursors.size(); ++i) {
            QString desc = tr("Курсор %1 (%2)").arg(i+1).arg(_verticalCursors[i].labelText);
            cursorMenu->addAction(tr("Редактировать %1...").arg(desc),
                                  this, [this, i]() { showEditVerticalCursorDialog(i); });
            cursorMenu->addAction(tr("Удалить %1").arg(desc),
                                  this, [this, i]() { removeVerticalCursor(i); });
        }
        cursorMenu->addSeparator();
        cursorMenu->addAction(tr("Удалить все курсоры"), this, &CustomPlotBase::clearVerticalCursors);
    }
}

void CustomPlotBase::showAxisRangeDialog(QCPAxis* axis) {
    if (!axis) return;

    bool isTimeAxis = dynamic_cast<QCPAxisTickerDateTime*>(axis->ticker().data()) != nullptr;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Диапазон оси"));
    QVBoxLayout* mainLayout = new QVBoxLayout(&dlg);

    QCheckBox* autoCheck = new QCheckBox(tr("Автомасштаб"));
    mainLayout->addWidget(autoCheck);

    QWidget* rangeWidget = new QWidget;
    QHBoxLayout* rangeLayout = new QHBoxLayout(rangeWidget);
    rangeLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(rangeWidget);

    // Переменные для захвата в лямбды
    QPointer<QDateTimeEdit> minTimeEdit, maxTimeEdit;
    QPointer<QDoubleSpinBox> minSpin, maxSpin;

    if (isTimeAxis) {
        minTimeEdit = new QDateTimeEdit;
        maxTimeEdit = new QDateTimeEdit;
        minTimeEdit->setDisplayFormat(DATE_TIME_FORMAT_WITHOUT_MS);
        maxTimeEdit->setDisplayFormat(DATE_TIME_FORMAT_WITHOUT_MS);
        minTimeEdit->setCalendarPopup(true);
        maxTimeEdit->setCalendarPopup(true);

        rangeLayout->addWidget(new QLabel(tr("От:")));
        rangeLayout->addWidget(minTimeEdit);
        rangeLayout->addWidget(new QLabel(tr("До:")));
        rangeLayout->addWidget(maxTimeEdit);

        // Текущий диапазон
        QCPRange range = axis->range();
        minTimeEdit->setDateTime(QDateTime::fromSecsSinceEpoch(range.lower));
        maxTimeEdit->setDateTime(QDateTime::fromSecsSinceEpoch(range.upper));

        auto applyFunc = [this, axis, autoCheck, minTimeEdit, maxTimeEdit, &dlg]() {
            if (autoCheck->isChecked()) {
                set_autorescale(true);
                axis->rescale();
            } else {
                set_autorescale(false);
                qint64 lower = minTimeEdit->dateTime().toSecsSinceEpoch();
                qint64 upper = maxTimeEdit->dateTime().toSecsSinceEpoch();
                if (lower < upper)
                    axis->setRange(lower, upper);
            }
            replot();
        };

        QObject::connect(autoCheck, &QCheckBox::toggled, rangeWidget, [rangeWidget](bool checked) {
            rangeWidget->setEnabled(!checked); // при авто поля ввода недоступны
        });
        QObject::connect(&dlg, &QDialog::accepted, applyFunc);
    } else {
        minSpin = new QDoubleSpinBox;
        maxSpin = new QDoubleSpinBox;
        minSpin->setRange(-1e9, 1e9);
        maxSpin->setRange(-1e9, 1e9);
        minSpin->setDecimals(6);
        maxSpin->setDecimals(6);

        rangeLayout->addWidget(new QLabel(tr("От:")));
        rangeLayout->addWidget(minSpin);
        rangeLayout->addWidget(new QLabel(tr("До:")));
        rangeLayout->addWidget(maxSpin);

        QCPRange range = axis->range();
        minSpin->setValue(range.lower);
        maxSpin->setValue(range.upper);

        auto applyFunc = [this, axis, autoCheck, minSpin, maxSpin, &dlg]() {
            if (autoCheck->isChecked()) {
                set_autorescale(true);
                axis->rescale();
            } else {
                set_autorescale(false);
                double lower = minSpin->value();
                double upper = maxSpin->value();
                if (lower < upper)
                    axis->setRange(lower, upper);
            }
            replot();
        };

        QObject::connect(autoCheck, &QCheckBox::toggled, rangeWidget, [rangeWidget](bool checked) {
            rangeWidget->setEnabled(!checked);
        });
        QObject::connect(&dlg, &QDialog::accepted, applyFunc);
    }

    autoCheck->setChecked(_autorescale);
    rangeWidget->setEnabled(!_autorescale);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    dlg.exec();
}

void CustomPlotBase::showAddThresholdDialog() {
    // Сбор видимых осей Y
    QList<QCPAxis*> yAxes;
    yAxes << yAxis;
    for (QCPAxis* ax : _rightAxes) {
        if (ax->visible()) yAxes << ax;
    }
    if (yAxes.isEmpty()) return;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Новая пороговая линия"));
    QFormLayout* form = new QFormLayout(&dlg);

    QComboBox* axisCombo = new QComboBox;
    for (QCPAxis* ax : yAxes)
        axisCombo->addItem(ax->label());
    form->addRow(tr("Ось:"), axisCombo);

    QDoubleSpinBox* valueSpin = new QDoubleSpinBox;
    valueSpin->setRange(-1e9, 1e9);
    valueSpin->setDecimals(4);
    form->addRow(tr("Значение:"), valueSpin);

    QPushButton* colorBtn = new QPushButton;
    QColor color = Qt::red;
    colorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
    form->addRow(tr("Цвет:"), colorBtn);
    QObject::connect(colorBtn, &QPushButton::clicked, &dlg, [&color, colorBtn]() {
        QColor newColor = QColorDialog::getColor(color, nullptr, tr("Цвет линии"));
        if (newColor.isValid()) {
            color = newColor;
            colorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
        }
    });

    QComboBox* styleCombo = new QComboBox;
    styleCombo->addItems({ tr("Сплошная"), tr("Пунктир"), tr("Тире"), tr("Штрих-пунктир") });
    form->addRow(tr("Стиль:"), styleCombo);

    QSpinBox* widthSpin = new QSpinBox;
    widthSpin->setRange(1, 10);
    widthSpin->setValue(1);
    form->addRow(tr("Толщина:"), widthSpin);

    QLineEdit* textEdit = new QLineEdit;
    form->addRow(tr("Метка:"), textEdit);
    textEdit->setText(QString("%1 %2").arg(QLocale::system().toString(valueSpin->value(), 'f', 3), axisCombo->currentText()));

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QCPAxis* axis = yAxes[axisCombo->currentIndex()];
        double value = valueSpin->value();
        Qt::PenStyle style = static_cast<Qt::PenStyle>(Qt::SolidLine + styleCombo->currentIndex()); // простой маппинг
        int width = widthSpin->value();
        QString text = textEdit->text();
        addThresholdLine(axis, value, color, style, width, text);
    }
}

void CustomPlotBase::showEditThresholdDialog(int index) {
    if (index < 0 || index >= _thresholdLines.size()) return;
    ThresholdLine& tl = _thresholdLines[index];

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Редактировать пороговую линию"));
    QFormLayout* form = new QFormLayout(&dlg);

    // Ось (только просмотр)
    QLineEdit* axisEdit = new QLineEdit(tl.axis->label());
    axisEdit->setReadOnly(true);
    form->addRow(tr("Ось:"), axisEdit);

    QDoubleSpinBox* valueSpin = new QDoubleSpinBox;
    valueSpin->setRange(-1e9, 1e9);
    valueSpin->setDecimals(4);
    valueSpin->setValue(tl.value);
    form->addRow(tr("Значение:"), valueSpin);

    QPushButton* colorBtn = new QPushButton;
    QColor color = tl.color;
    colorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
    form->addRow(tr("Цвет:"), colorBtn);
    QObject::connect(colorBtn, &QPushButton::clicked, &dlg, [&color, colorBtn]() {
        QColor newColor = QColorDialog::getColor(color, nullptr, tr("Цвет линии"));
        if (newColor.isValid()) {
            color = newColor;
            colorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
        }
    });

    QComboBox* styleCombo = new QComboBox;
    styleCombo->addItems({ tr("Сплошная"), tr("Пунктир"), tr("Тире"), tr("Штрих-пунктир") });
    styleCombo->setCurrentIndex(tl.style - Qt::SolidLine);
    form->addRow(tr("Стиль:"), styleCombo);

    QSpinBox* widthSpin = new QSpinBox;
    widthSpin->setRange(1, 10);
    widthSpin->setValue(tl.width);
    form->addRow(tr("Толщина:"), widthSpin);

    QLineEdit* textEdit = new QLineEdit(tl.text);
    form->addRow(tr("Метка:"), textEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton* deleteBtn = buttons->addButton(tr("Удалить"), QDialogButtonBox::DestructiveRole);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(deleteBtn, &QPushButton::clicked, &dlg, [&]() {
        removeThresholdLine(index);
        dlg.reject();
    });

    if (dlg.exec() == QDialog::Accepted) {
        tl.value = valueSpin->value();
        tl.color = color;
        tl.style = static_cast<Qt::PenStyle>(Qt::SolidLine + styleCombo->currentIndex());
        tl.width = widthSpin->value();
        tl.text = textEdit->text();

        // Обновить графические элементы
        if (tl.line) {
            tl.line->setPen(QPen(tl.color, tl.width, tl.style));
            // Обновим координаты сразу
            QCPRange xRange = xAxis->range();
            tl.line->point1->setCoords(xRange.lower, tl.value);
            tl.line->point2->setCoords(xRange.upper, tl.value);
        }
        if (tl.label) {
            tl.label->setText(tl.text);
            if (tl.text.isEmpty())
                tl.label->setVisible(false);
            else
                tl.label->setVisible(true);
            QCPRange yRange = tl.axis->range();
            double offset = (yRange.upper - yRange.lower) * 0.02;
            if (qFuzzyIsNull(offset)) offset = 1.0;
            tl.label->position->setCoords(xAxis->range().upper, tl.value + offset);
            tl.label->setPen(QPen(tl.color));
        }
        replot();
        saveThresholdLines();
    }
}

void CustomPlotBase::showAddVerticalCursorDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Новый вертикальный курсор"));
    QFormLayout* form = new QFormLayout(&dlg);

    QDateTimeEdit* dateEdit = new QDateTimeEdit;
    dateEdit->setDisplayFormat(DATE_TIME_FORMAT_WITHOUT_MS);
    dateEdit->setDateTime(QDateTime::currentDateTime());
    form->addRow(tr("Время:"), dateEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        double x = dateEdit->dateTime().toSecsSinceEpoch();
        addVerticalCursor(x, Qt::blue, 1);
    }
}

void CustomPlotBase::showEditVerticalCursorDialog(int index) {
    if (index < 0 || index >= _verticalCursors.size()) return;
    VerticalCursor& vc = _verticalCursors[index];

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Редактировать курсор"));
    QFormLayout* form = new QFormLayout(&dlg);

    QDateTimeEdit* dateEdit = new QDateTimeEdit;
    dateEdit->setDisplayFormat(DATE_TIME_FORMAT_WITHOUT_MS);
    dateEdit->setDateTime(QDateTime::fromSecsSinceEpoch(vc.xValue));
    form->addRow(tr("Время:"), dateEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton* deleteBtn = buttons->addButton(tr("Удалить"), QDialogButtonBox::DestructiveRole);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(deleteBtn, &QPushButton::clicked, &dlg, [&]() { removeVerticalCursor(index); dlg.reject(); });

    if (dlg.exec() == QDialog::Accepted) {
        double newX = dateEdit->dateTime().toSecsSinceEpoch();
        if (newX != vc.xValue) {
            vc.xValue = newX;
            vc.labelText = dateEdit->dateTime().toString(DATE_TIME_FORMAT_WITHOUT_MS);
            if (vc.line) {
                vc.line->point1->setCoords(newX, vc.line->point1->coords().y());
                vc.line->point2->setCoords(newX, vc.line->point2->coords().y());
            }
            if (vc.label) {
                vc.label->setText(vc.labelText);
                vc.label->position->setCoords(newX, vc.label->position->coords().y());
            }
            replot();
            saveVerticalCursors();
        }
    }
}
