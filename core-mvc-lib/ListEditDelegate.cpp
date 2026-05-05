#include "ListEditDelegate.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "CoreConfig.h"
#include "DynamicListWidget.h"

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
    // Создаём диалог, в который поместим DynamicListWidget
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(tr("Редактирование списка"));
    dialog->setModal(true);

    DynamicListWidget *listWidget = new DynamicListWidget(dialog);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    if (okButton) okButton->setText("Ок");
    if (cancelButton) cancelButton->setText("Отмена");

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(listWidget);
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

    DynamicListWidget *listWidget = dialog->findChild<DynamicListWidget*>();
    if (!listWidget) return;

    // Получаем текущий список строк из модели
    QString text = index.data(Qt::EditRole).toString();
    QStringList items = splitStringToList(text);
    listWidget->setItems(items);
}

void ListEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QDialog *dialog = qobject_cast<QDialog*>(editor);
    if (!dialog) return;

    // Если пользователь нажал OK
    if (dialog->result() == QDialog::Accepted) {
        DynamicListWidget *listWidget = dialog->findChild<DynamicListWidget*>();
        if (listWidget) {
            QStringList items = listWidget->getItems();
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
