#include "CalendarWidget.h"

#include "Logger.h"

CalendarWidget::CalendarWidget(QWidget* parent) :
    QCalendarWidget(parent)
{
    // Отключаем стандартное выделение дат (будем обрабатывать вручную)
    setSelectionMode(QCalendarWidget::NoSelection);

    // Подключаем обработчик клика по дате
    connect(this, &QCalendarWidget::clicked, this, &CalendarWidget::handleDateClicked);
}

// Сбросить выделение
void CalendarWidget::resetSelection() {
    _startDate = _endDate = QDate();
    clearHighlight();
    update();
}

void CalendarWidget::setSelection(const QDate& start, const QDate& end) {
    _startDate = start;
    _endDate = end;
    highlightRange();
    if (_startDate.isValid()) {
        this->setSelectedDate(_startDate);
    } else {
        this->setSelectedDate(QDate::currentDate());
    }
}

void CalendarWidget::handleDateClicked(const QDate& date) {
    if (!_startDate.isValid() || (_startDate.isValid() && _endDate.isValid())) {
        // Первый клик или сброс после двух кликов
        _startDate = date;
        _endDate = QDate();
    } else {
        // Второй клик (устанавливаем конечную дату)
        _endDate = date;
        // Если end < start, меняем местами
        if (_endDate < _startDate) std::swap(_startDate, _endDate);
    }

    // Обновляем визуальное выделение
    highlightRange();
    emit selectionChanged(_startDate, _endDate);
}

void CalendarWidget::highlightRange() {
    clearHighlight();
    if (!_startDate.isValid()) return;

    QTextCharFormat format;
    format.setBackground(Qt::yellow);  // Цвет фона для выделения
    format.setForeground(Qt::black);

    // Если выбрана одна дата
    if (!_endDate.isValid()) {
        setDateTextFormat(_startDate, format);
        return;
    }

    // Выделяем диапазон
    for (QDate d = _startDate; d <= _endDate; d = d.addDays(1)) {
        setDateTextFormat(d, format);
    }
}

void CalendarWidget::clearHighlight() {
    // Сбрасываем формат всех дат
    setDateTextFormat(QDate(), QTextCharFormat());
}
