#include "ListEditDelegate.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <core-lib/CoreConfig.h>
#include <core-widgets-lib/DynamicListPanel.h>

static QStringList splitStringToList(const QString &str)
{
    if (str.isEmpty())
        return QStringList();
    // Разделяем по ';', игнорируем пустые части, обрезаем пробелы
    QStringList list = str.split(ELEMENTS_DELIMETER, Qt::SkipEmptyParts);
    for (QString &s : list)
        s = s.trimmed();
    return list;
}

static QString joinListToString(const QStringList &list)
{
    return list.join(ELEMENTS_DELIMETER);
}

ListEditDelegate::ListEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ListEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    // Создаём диалог, в который поместим DynamicListPanel
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(tr("Редактирование списка"));
    dialog->setModal(true);

    DynamicListPanel *dlp = new DynamicListPanel(dialog);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    if (okButton) okButton->setText("Ок");
    if (cancelButton) cancelButton->setText("Отмена");

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(dlp);
    layout->addWidget(buttonBox);
    dialog->setLayout(layout);

    // Сохраняем указатель на виджет, чтобы потом получить данные
    // Можно также использовать свойства диалога
    return dialog;
}

void ListEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDialog *dialog = qobject_cast<QDialog*>(editor);
    if (!dialog) return;

    DynamicListPanel *dlp = dialog->findChild<DynamicListPanel*>();
    if (!dlp) return;

    // Получаем текущий список строк из модели
    QString text = index.data(Qt::EditRole).toString();
    QStringList items = splitStringToList(text);
    dlp->setItems(items);
}

void ListEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QDialog *dialog = qobject_cast<QDialog*>(editor);
    if (!dialog) return;

    // Если пользователь нажал OK
    if (dialog->result() == QDialog::Accepted) {
        DynamicListPanel *dlp = dialog->findChild<DynamicListPanel*>();
        if (dlp) {
            QStringList items = dlp->items();
            QString joined = joinListToString(items);
            model->setData(index, QVariant(joined), Qt::EditRole);
        }
    }
}

void ListEditDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    // Диалог отображается по центру, не нужно позиционировать относительно ячейки
    // Можно задать начальный размер
    editor->resize(400, 500);
}
