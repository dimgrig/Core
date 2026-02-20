#include "DoubleSpinBoxDelegate.h"

#include <QDoubleSpinBox>

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject *)
{
    this->setDecimals(3);
}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setDecimals(this->_decimals);
    editor->setMinimum(0);
    editor->setMaximum(99999999999999.999);

    return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *dsb = static_cast<QDoubleSpinBox*>(editor);

    dsb->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDoubleSpinBox *dsb = static_cast<QDoubleSpinBox*>(editor);

    double value = dsb->value();

    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void DoubleSpinBoxDelegate::setDecimals(int decimals)
{
    this->_decimals = decimals;
}

int DoubleSpinBoxDelegate::getDecimals()
{
    return this->_decimals;
}
