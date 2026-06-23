#ifndef FILTERHEADER_H
#define FILTERHEADER_H

#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QWidgetAction>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT FilterHeader : public QHeaderView {
    Q_OBJECT
public:
    FilterHeader(QWidget* parent = nullptr);

    void setFilterEnabled(int section, bool enabled);
    bool isFilterEnabled(int section) const;

    void set_date(int section) {this->dates.insert(section);};
    void set_datetime(int section) {this->datetimes.insert(section);};
    void set_log(int section) {this->logs.insert(section);};

    void registerCustomFilter(int section,
                              std::function<QWidget*()> widgetFactory,
                              std::function<void(QWidget*)> onApply);

    void setFilter(int section, const QString& text);
    QString getFilter(int section);
signals:
    void filterChanged(int section, const QString& text);
private slots:
    void onFilterTextChanged(int section, const QString& text);
    void clearFilter(int section);
    void clearAllFilters();
protected:
    //void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
private:
    void showFilterMenu(int section, const QPoint& pos);
    QString createDateFilter(const QDate& startDate, const QDate& endDate);
    QString createDateTimeFilter(const QDateTime& startDateTime, const QDateTime& endDateTime);
    void getDateSelection(QString filter, QDate& startDate, QDate& endDate);
    void getDateTimeSelection(QString filter, QDateTime& startDateTime, QDateTime& endDateTime);
    QString checkLogLevelToText(int section, QString& text);
    QString checkLogLevelFromText(int section, QString& text);

    QSet<int> disabledSections;   // колонки, для которых фильтрация запрещена

    // Хранилище фильтров: [column, filter text]
    QMap<int, QString> filters;
    QSet<int> dates;
    QSet<int> datetimes;
    QSet<int> logs;

    QMap<int, std::function<QWidget*()>>         customWidgetFactories;
    QMap<int, std::function<void(QWidget*)>>     customApplyHandlers;
};

#endif // FILTERHEADER_H
