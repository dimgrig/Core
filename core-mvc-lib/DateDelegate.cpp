#include "DateDelegate.h"

DateDelegate::DateDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

// Создаем редактор (QDateEdit с календарем)
QWidget* DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QDateEdit *editor = new QDateEdit(parent);
    editor->setDisplayFormat("dd.MM.yyyy"); // Формат даты
    editor->setCalendarPopup(true); // Включаем календарь
    editor->setDate(QDate::currentDate());
    return editor;
}

// Устанавливаем текущее значение даты в редактор
void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QDate date = index.data(Qt::EditRole).toDate();
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    dateEdit->setDate(date);
}

// Сохраняем значение из редактора в модель
void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    model->setData(index, dateEdit->date(), Qt::EditRole);
}

// Опционально: форматируем отображение даты в ячейке
void DateDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QDate date = index.data(Qt::DisplayRole).toDate();
    QString text = date.toString("dd.MM.yyyy"); // Формат отображения
    painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
}
