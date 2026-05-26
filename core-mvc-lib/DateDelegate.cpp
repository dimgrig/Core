#include "DateDelegate.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

DateDelegate::DateDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

// Создаем редактор (QDateEdit с календарем)
QWidget* DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    // QDateEdit *editor = new QDateEdit(parent);
    // editor->setDisplayFormat("dd.MM.yyyy"); // Формат даты
    // editor->setCalendarPopup(true); // Включаем календарь
    // editor->setDate(QDate::currentDate());
    // return editor;

    // Создаём диалог, в который поместим QDateEdit
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(tr("Редактирование списка"));
    dialog->setModal(true);

    QDateEdit *dateEdit = new QDateEdit(parent);
    dateEdit->setDisplayFormat("dd.MM.yyyy"); // Формат даты
    dateEdit->setCalendarPopup(true); // Включаем календарь
    dateEdit->setDate(QDate::currentDate());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    if (okButton) okButton->setText("Ок");
    if (cancelButton) cancelButton->setText("Отмена");

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(dateEdit);
    layout->addWidget(buttonBox);
    dialog->setLayout(layout);

    // Сохраняем указатель на виджет, чтобы потом получить данные
    // Можно также использовать свойства диалога
    return dialog;
}

// Устанавливаем текущее значение даты в редактор
void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(index);
    // QDate date = index.data(Qt::EditRole).toDate();
    // QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    // dateEdit->setDate(date);

    QDialog *dialog = qobject_cast<QDialog*>(editor);
    if (!dialog) return;

    QDateEdit *dateEdit = dialog->findChild<QDateEdit*>();
    if (!dateEdit) return;

    QDate date = index.data(Qt::EditRole).toDate();
    dateEdit->setDate(date);
}

// Сохраняем значение из редактора в модель
void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    // QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    // model->setData(index, dateEdit->date(), Qt::EditRole);
    QDialog *dialog = qobject_cast<QDialog*>(editor);
    if (!dialog) return;

    // Если пользователь нажал OK
    if (dialog->result() == QDialog::Accepted) {
        QDateEdit *dateEdit = dialog->findChild<QDateEdit*>();
        if (dateEdit) {
            model->setData(index, dateEdit->date(), Qt::EditRole);
        }
    }
}

// // Опционально: форматируем отображение даты в ячейке
// void DateDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
//     QDate date = index.data(Qt::DisplayRole).toDate();
//     QString text = date.toString("dd.MM.yyyy"); // Формат отображения
//     painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
// }

QString DateDelegate::displayText(const QVariant &value, const QLocale &locale) const {
    QDate date = value.toDate();
    if (date.isValid())
        return date.toString("dd.MM.yyyy");
    return QStyledItemDelegate::displayText(value, locale);
}

