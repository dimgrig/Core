#ifndef DATETIMERANGEWIDGET_H
#define DATETIMERANGEWIDGET_H

#include <QDateTimeEdit>
#include <QPushButton>
#include <QTextCharFormat>

#include <core-lib/CoreConfig.h>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT DateTimeRangeWidget : public QWidget {
    Q_OBJECT
public:
    DateTimeRangeWidget(QWidget* parent = nullptr);

    // Сбросить выделение
    void resetSelection();

    // Получить выбранные даты
    QDateTime startDateTime() const { return m_startEdit->dateTime(); }
    QDateTime endDateTime() const { return m_endEdit->dateTime(); }

    void set_format(QString format) {_format = format;};
    QString get_format() const {return _format;};
signals:
    void selectionChanged(const QDateTime& start, const QDateTime& end);
public slots:
    void setSelection(const QDateTime& start, const QDateTime& end);
private:

    QDateTimeEdit *m_startEdit;
    QDateTimeEdit *m_endEdit;

    // QPushButton *m_applyButton;
    // QPushButton *m_resetButton;

    QString _format = DATE_TIME_FORMAT_WITHOUT_MS;
};

#endif // DATETIMERANGEWIDGET_H
