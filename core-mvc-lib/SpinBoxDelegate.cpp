#include "SpinBoxDelegate.h"

#include <QSpinBox>

SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QSpinBox *editor = new QSpinBox(parent);

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QSpinBox *sb = static_cast<QSpinBox*>(editor);

    sb->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *sb = static_cast<QSpinBox*>(editor);

    double value = sb->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
