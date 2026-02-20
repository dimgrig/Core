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
    void set_date(int column) {this->dates.insert(column);};

signals:
    void filterChanged(int column, const QString& text);
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
    void getDateSelection(QString filter, QDate& startDate, QDate& endDate);

    // Хранилище фильтров: [column, filter text]
    QMap<int, QString> filters;
    QSet<int> dates;
};

#endif // FILTERHEADER_H
