#include "LineEditDelegate.h"

#include <QLineEdit>

LineEditDelegate::LineEditDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

QWidget * LineEditDelegate::createEditor(QWidget *parent,
                                  const QStyleOptionViewItem & option,
                                  const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QLineEdit *editor = new QLineEdit(parent);

    return editor;
}

void LineEditDelegate::setEditorData(QWidget *editor,
                               const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit * lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);

    // // Deselect text.
    // lineEdit->deselect();

    // // Move the cursor to the beginning.
    // lineEdit->setCursorPosition(0);
}

void LineEditDelegate::setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const
{
    QLineEdit * lineEdit = static_cast<QLineEdit*>(editor);

    QString value = lineEdit->text();

    model->setData(index, value, Qt::EditRole);
}
