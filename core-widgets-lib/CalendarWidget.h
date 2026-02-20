#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QCalendarWidget>
#include <QDate>
#include <QTextCharFormat>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT CalendarWidget : public QCalendarWidget {
    Q_OBJECT
public:
    CalendarWidget(QWidget* parent = nullptr);

    // Сбросить выделение
    void resetSelection();

    // Получить выбранные даты
    QDate startDate() const { return _startDate; }
    QDate endDate() const { return _endDate; }

signals:
    void selectionChanged(const QDate& start, const QDate& end);
public slots:
    void setSelection(const QDate& start, const QDate& end);
private slots:
    void handleDateClicked(const QDate& date);
private:
    void highlightRange();
    void clearHighlight();

    QDate _startDate, _endDate;
};

#endif // CALENDARWIDGET_H
