#include "DateTimeRangeWidget.h"

#include <QHBoxLayout>
#include <QLabel>

#include "Logger.h"

DateTimeRangeWidget::DateTimeRangeWidget(QWidget* parent) :
    QWidget(parent)
{
    m_startEdit = new QDateTimeEdit(this);
    m_endEdit = new QDateTimeEdit(this);

    // Настройка формата отображения
    m_startEdit->setDisplayFormat(_format);
    m_endEdit->setDisplayFormat(_format);

    // Включаем календарь
    m_startEdit->setCalendarPopup(true);
    m_endEdit->setCalendarPopup(true);

    // Устанавливаем время по умолчанию (начало сегодня, конец сегодня+1)
    QDateTime now = QDateTime::currentDateTime();
    m_startEdit->setDateTime(now.addDays(-1));
    m_endEdit->setDateTime(now);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout *startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel("Начало:", this));
    startLayout->addWidget(m_startEdit);
    mainLayout->addLayout(startLayout);

    QHBoxLayout *endLayout = new QHBoxLayout();
    endLayout->addWidget(new QLabel("Конец:", this));
    endLayout->addWidget(m_endEdit);
    mainLayout->addLayout(endLayout);

    // m_applyButton = new QPushButton("Применить", this);
    // m_resetButton = new QPushButton("Сбросить", this);

    // QHBoxLayout *buttonLayout = new QHBoxLayout();
    // buttonLayout->addWidget(m_applyButton);
    // buttonLayout->addWidget(m_resetButton);
    // mainLayout->addLayout(buttonLayout);

    // // Соединения
    // connect(m_applyButton, &QPushButton::clicked, [this]() {
    //     emit selectionChanged(m_startEdit->dateTime(), m_endEdit->dateTime());
    // });

    // connect(m_resetButton, &QPushButton::clicked, this, &DateTimeRangeWidget::resetSelection);
}

// Сбросить выделение
void DateTimeRangeWidget::resetSelection() {
    // Сброс: очищаем или устанавливаем пустые QDateTime (невалидные)
    m_startEdit->setDateTime(QDateTime());
    m_endEdit->setDateTime(QDateTime());
    emit selectionChanged(QDateTime(), QDateTime());
}

void DateTimeRangeWidget::setSelection(const QDateTime& start, const QDateTime& end) {
    if (start.isValid())
        m_startEdit->setDateTime(start);
    if (end.isValid())
        m_endEdit->setDateTime(end);
}
