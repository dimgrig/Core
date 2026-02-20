#ifndef DOUBLESPINBOXDELEGATE_H
#define DOUBLESPINBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QDateEdit>
#include <QPainter>

#include "core-mvc-lib-global.h"

CORE_MVC_LIB_EXPORT class DoubleSpinBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit DoubleSpinBoxDelegate(QObject *parent = nullptr);

    // Создаем редактор (QDateEdit с календарем)
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Устанавливаем текущее значение даты в редактор
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    // Сохраняем значение из редактора в модель
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override ;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const;

    void setDecimals(int decimals);
    int getDecimals();
private:
    int _decimals;
};

#endif // DOUBLESPINBOXDELEGATE_H
