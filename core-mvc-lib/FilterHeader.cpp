#include "FilterHeader.h"

#include <QApplication>
#include <QCalendarWidget>
#include <QDateTimeEdit>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

#include "CalendarWidget.h"
#include "CoreConfig.h"
#include "DateTimeRangeWidget.h"
#include "Logger.h"

FilterHeader::FilterHeader(const QString &objectName, QWidget* parent) :
    QHeaderView(Qt::Horizontal, parent)
{
    this->setObjectName(objectName);

    setSectionsClickable(true);
    setHighlightSections(true);

    this->restoreFilters();
}

FilterHeader::~FilterHeader() {
    this->saveFilters();
}

void FilterHeader::saveFilters() const {
    QSettings settings;
    settings.beginGroup(this->objectName());
    settings.remove(""); // очищаем предыдущие ключи в этой группе
    for (auto it = filters.cbegin(); it != filters.cend(); ++it) {
        settings.setValue(QString::number(it.key()), it.value());
    }
    settings.endGroup();
}

void FilterHeader::restoreFilters() {
    QSettings settings;
    settings.beginGroup(this->objectName());
    QMap<int, QString> saved;
    const QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        bool ok;
        int col = key.toInt(&ok);
        if (ok)
            saved.insert(col, settings.value(key).toString());
    }
    settings.endGroup();

    for (auto it = saved.cbegin(); it != saved.cend(); ++it) {
        setFilter(it.key(), it.value()); // внутри уже вызывает QTimer::singleShot и фильтрацию
    }
}

void FilterHeader::setFilterEnabled(int section, bool enabled) {
    if (enabled)
        disabledSections.remove(section);
    else
        disabledSections.insert(section);
    viewport()->update(); // перерисовка иконок
}

bool FilterHeader::isFilterEnabled(int section) const {
    return !disabledSections.contains(section);
}

void FilterHeader::registerCustomFilter(int section,
                          std::function<QWidget*()> widgetFactory,
                          std::function<void(QWidget*)> onApply) {
    customWidgetFactories[section] = widgetFactory;
    customApplyHandlers[section]   = onApply;
}

void FilterHeader::setFilter(int section, const QString& text) {
    if (text.isEmpty()) {
        filters.remove(section);
    } else {
        filters[section] = text;
    }
    QTimer::singleShot(100, [=](){
        emit filterChanged(section, text);
        viewport()->update();
    });
}

QString FilterHeader::getFilter(int section) {
    if (filters.count(section)) {
        return filters[section];
    }
    return "";
}

void FilterHeader::mouseDoubleClickEvent(QMouseEvent* event) {
    int section = logicalIndexAt(event->pos());
    if (section == -1)
        return;

    if (!isFilterEnabled(section))
        return;

    if (event->button() == Qt::LeftButton) {
        showFilterMenu(section, event->globalPos());
    } else {
        QHeaderView::mouseDoubleClickEvent(event);
    }
}

void FilterHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
    painter->save();

    // // Если фильтр активен, заливаем фон другим цветом
    // if (filters.contains(logicalIndex) && isFilterEnabled(logicalIndex)) {
    //     painter->fillRect(rect.adjusted(1, 1, -1, -1), QColor(220, 240, 255)); // светло-голубой
    // }

    // Стандартная прорисовка секции
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    // Рисуем иконку фильтра, если он применен
    if (filters.contains(logicalIndex) && isFilterEnabled(logicalIndex)) {
        painter->save();
        painter->setPen(m_iconColor);
        painter->setFont(QFont("Arial", 14, QFont::Bold));
        QRect iconRect = rect.adjusted(rect.width() - 24, 0, -4, 0);
        painter->drawText(iconRect, Qt::AlignRight | Qt::AlignVCenter, "▼");
        painter->restore();
    }
}

void FilterHeader::showFilterMenu(int section, const QPoint& pos) {
    // ---------- Кастомный виджет ----------
    if (customWidgetFactories.contains(section)) {
        QWidget *filterWidget = customWidgetFactories[section](); // создаём без родителя
        QMenu menu;

        QWidget *container = new QWidget(&menu);
        QVBoxLayout *vlay = new QVBoxLayout(container);
        vlay->addWidget(filterWidget);   // теперь filterWidget без родителя — ошибки нет

        QPushButton *applyBtn = new QPushButton(tr("Применить"), container);
        QPushButton *clearBtn = new QPushButton(tr("Очистить"), container);
        QHBoxLayout *btnLay = new QHBoxLayout();
        btnLay->addWidget(applyBtn);
        btnLay->addWidget(clearBtn);
        vlay->addLayout(btnLay);

        QWidgetAction *action = new QWidgetAction(&menu);
        action->setDefaultWidget(container);
        menu.addAction(action);

        // Кнопка "Применить"
        auto applyHandler = customApplyHandlers[section];
        connect(applyBtn, &QPushButton::clicked, [this, section, filterWidget, applyHandler, &menu]() {
            menu.close();
            applyHandler(filterWidget);   // вызываем колбек, передавая временный виджет
        });

        // Кнопка "Очистить"
        connect(clearBtn, &QPushButton::clicked, [this, section, &menu]() {
            menu.close();
            clearFilter(section);         // сброс фильтра через стандартный механизм
        });

        menu.exec(pos);
        filterWidget->deleteLater();      // удаляем временный виджет после закрытия меню
        return;
    }

    QMenu menu;

    QWidget* widget = new QWidget(&menu);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    QPushButton* applyButton = new QPushButton("Применить", widget);

    if (this->dates.contains(section)) {
        QPushButton* resetButton = new QPushButton("Сбросить", widget);

        //дата
        CalendarWidget* calendar = new CalendarWidget(widget);
        {
            QDate startDate;
            QDate endDate;
            getDateSelection(filters.value(section), startDate, endDate);
            calendar->setSelection(startDate, endDate);
        }
        calendar->setCurrentPage(QDate::currentDate().year(), QDate::currentDate().month());
        calendar->setSelectionMode(QCalendarWidget::SingleSelection);

        layout->addWidget(calendar);
        layout->addWidget(applyButton);
        layout->addWidget(resetButton);

        connect(applyButton, &QPushButton::clicked, [&]() {
            menu.close();

            // auto selected = calendar->selectedDate();
            // QString selected_str = selected.toString(ISO_DATE_FORMAT);
            // onFilterTextChanged(section, selected_str);

            QDate start = calendar->startDate();
            QDate end = calendar->endDate();
            QString date_filter = this->createDateFilter(start, end);
            onFilterTextChanged(section, date_filter);
        });

        connect(resetButton, &QPushButton::clicked, [&]() {
            menu.close();
            clearFilter(section);
        });
    } else if (this->datetimes.contains(section)) {
        QPushButton* resetButton = new QPushButton("Сбросить", widget);

        DateTimeRangeWidget* dtrw = new DateTimeRangeWidget(widget);
        {
            QDateTime startDateTime;
            QDateTime endDateTime;
            getDateTimeSelection(filters.value(section), startDateTime, endDateTime);
            dtrw->setSelection(startDateTime, endDateTime);
        }

        layout->addWidget(dtrw);
        layout->addWidget(applyButton);
        layout->addWidget(resetButton);

        connect(applyButton, &QPushButton::clicked, [&]() {
            menu.close();

            QDateTime start = dtrw->startDateTime();
            QDateTime end = dtrw->endDateTime();
            QString date_time_filter = this->createDateTimeFilter(start, end);
            onFilterTextChanged(section, date_time_filter);
        });

        connect(resetButton, &QPushButton::clicked, [&]() {
            menu.close();
            clearFilter(section);
        });
    } else {
        // Создаем виджет с полем ввода и кнопкой
        QLineEdit* filterEdit = new QLineEdit(widget);

        QString text = filters.value(section);
        text = checkLogLevelToText(section, text);
        filterEdit->setText(text);
        layout->addWidget(filterEdit);
        layout->addWidget(applyButton);

        {
            //cross for QLineEdit

            // Добавляем иконку крестика справа
            QAction* clearAction = filterEdit->addAction(
                QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
                QLineEdit::TrailingPosition
                );

            clearAction->setVisible(!filterEdit->text().isEmpty());

            // Очищаем текст при клике
            connect(clearAction, &QAction::triggered, filterEdit, &QLineEdit::clear);

            // Скрываем иконку, когда текста нет
            connect(filterEdit, &QLineEdit::textChanged, [filterEdit, clearAction](const QString& text) {
                clearAction->setVisible(!text.isEmpty());
            });

            connect(filterEdit, &QLineEdit::returnPressed, applyButton, &QPushButton::click);
            // Применяем фильтр и закрываем меню по нажатию кнопки
            connect(applyButton, &QPushButton::clicked, [&]() {
                menu.close();
                QString text = filterEdit->text();
                text = checkLogLevelFromText(section, text);
                onFilterTextChanged(section, text);
            });                      
        }
    }

    // Добавляем виджет через QWidgetAction
    QWidgetAction* action = new QWidgetAction(&menu);
    action->setDefaultWidget(widget);
    menu.addAction(action);

    menu.exec(pos);
}

void FilterHeader::onFilterTextChanged(int section, const QString& text) {
    this->setFilter(section, text);
}

void FilterHeader::clearFilter(int section) {
    filters.remove(section);
    QTimer::singleShot(100, [=](){
        emit filterChanged(section, "");
        viewport()->update();
    });
}

void FilterHeader::clearAllFilters() {
    filters.clear();
    QTimer::singleShot(100, [=](){
        emit filterChanged(-1, "");
        viewport()->update();
    });
}

QString FilterHeader::createDateFilter(const QDate& startDate, const QDate& endDate) {
    // if (startDate.isValid() && endDate.isValid()) {
    //     return QString("%1 BETWEEN '%2' AND '%3'")
    //         .arg(columnName)
    //         .arg(startDate.toString(ISO_DATE_FORMAT))
    //         .arg(endDate.toString(ISO_DATE_FORMAT));
    // } else if (startDate.isValid()) {
    //     return QString("%1 >= '%2'")
    //         .arg(columnName)
    //         .arg(startDate.toString(ISO_DATE_FORMAT));
    // } else if (endDate.isValid()) {
    //     return QString("%1 <= '%2'")
    //         .arg(columnName)
    //         .arg(endDate.toString(ISO_DATE_FORMAT));
    // }
    if (startDate.isValid() && endDate.isValid()) {
        return QString("BETWEEN '%1' AND '%2'")
            .arg(startDate.toString(ISO_DATE_FORMAT))
            .arg(endDate.toString(ISO_DATE_FORMAT));
    } else if (startDate.isValid()) {
        return QString(" = '%1'")
            .arg(startDate.toString(ISO_DATE_FORMAT));
    }
    return QString(); // Пустой фильтр, если даты не заданы
}

QString FilterHeader::createDateTimeFilter(const QDateTime& startDateTime, const QDateTime& endDateTime) {
    if (startDateTime.isValid() && endDateTime.isValid()) {
        return QString("BETWEEN '%1' AND '%2'")
        .arg(startDateTime.toString(DATE_TIME_FORMAT_WITHOUT_MS))
            .arg(endDateTime.toString(DATE_TIME_FORMAT_WITHOUT_MS));
    } else if (startDateTime.isValid()) {
        return QString(" >= '%1'")
        .arg(startDateTime.toString(DATE_TIME_FORMAT_WITHOUT_MS));
    } else if (endDateTime.isValid()) {
        return QString(" <= '%1'")
        .arg(endDateTime.toString(DATE_TIME_FORMAT_WITHOUT_MS));
    }
    return QString();
}

void FilterHeader::getDateSelection(QString filter, QDate& startDate, QDate& endDate) {
    if (filter.contains("BETWEEN")) {
        filter.remove(0, 8);
        filter.replace("'", "");
        QStringList date_sl = filter.split(" AND ");
        if (date_sl.size() == 2) {
            startDate = QDate::fromString(date_sl.front(), ISO_DATE_FORMAT);
            endDate = QDate::fromString(date_sl.back(), ISO_DATE_FORMAT);
        }
    } else if (filter.contains(" = ")) {
        filter.remove(0, 3);
        filter.replace("'", "");
        startDate = QDate::fromString(filter, ISO_DATE_FORMAT);
    }
}

void FilterHeader::getDateTimeSelection(QString filter, QDateTime& startDateTime, QDateTime& endDateTime) {
    if (filter.contains("BETWEEN")) {
        filter.remove(0, 8); // убираем "BETWEEN "
        filter.replace("'", "");
        QStringList date_time_sl = filter.split(" AND ");
        if (date_time_sl.size() == 2) {
            startDateTime = QDateTime::fromString(date_time_sl.front(), DATE_TIME_FORMAT_WITHOUT_MS);
            endDateTime = QDateTime::fromString(date_time_sl.back(), DATE_TIME_FORMAT_WITHOUT_MS);
        }
    } else if (filter.contains(" >= ")) {
        filter.remove(0, 4);
        filter.replace("'", "");
        startDateTime = QDateTime::fromString(filter, DATE_TIME_FORMAT_WITHOUT_MS);
    } else if (filter.contains(" <= ")) {
        filter.remove(0, 4);
        filter.replace("'", "");
        endDateTime = QDateTime::fromString(filter, DATE_TIME_FORMAT_WITHOUT_MS);
    }
}

QString FilterHeader::checkLogLevelToText(int section, QString& text) {
    if (this->logs.contains(section)) {
        if (text != "") {
            QString str = logLevelToString(static_cast<LOG_LEV>(text.toInt()));
            if (str == "Unknown") {
                text = "";
            } else {
                text = str;
            }
        }
    }
    return text;
}

QString FilterHeader::checkLogLevelFromText(int section, QString& text) {
    if (this->logs.contains(section)) {
        if (text != "") {
            auto level = logLevelFromString(text);
            if (level == LOG_LEV::NONE_LOG) {
                text = "";
            } else {
                text = QString::number(static_cast<int>(level));
            }
        }
    }
    return text;
}
