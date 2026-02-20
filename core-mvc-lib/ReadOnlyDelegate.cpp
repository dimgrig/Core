#include "ReadOnlyDelegate.h"

ReadOnlyDelegate::ReadOnlyDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

bool ReadOnlyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    Q_UNUSED(event);
    Q_UNUSED(model);
    Q_UNUSED(option);
    Q_UNUSED(index);
    return false;
}

QWidget* ReadOnlyDelegate::ReadOnlyDelegate::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const {
    return Q_NULLPTR;
}
