#ifndef READONLYDELEGATE_H
#define READONLYDELEGATE_H

#include <QStyledItemDelegate>
#include <QDateEdit>
#include <QPainter>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT ReadOnlyDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ReadOnlyDelegate(QObject *parent = nullptr);
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QWidget* createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
};

#endif // READONLYDELEGATE_H
