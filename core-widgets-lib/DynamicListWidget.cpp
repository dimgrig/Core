#include "DynamicListWidget.h"

#include "aqp.h"

#include <QMenu>
#include <QAction>
#include <QInputDialog>

DynamicListWidget::DynamicListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setupContextMenu();
    // Отключаем встроенное редактирование, чтобы редактирование шло только через меню
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &QListWidget::doubleClicked, this, &DynamicListWidget::onEditItem);
}

QStringList DynamicListWidget::getItems() const
{
    QStringList result;
    for (int i = 0; i < count(); ++i) {
        result << item(i)->text();
    }
    return result;
}

void DynamicListWidget::setItems(const QStringList &items)
{
    clear();
    for (const QString &text : items) {
        addItem(text);
    }
}

void DynamicListWidget::setupContextMenu()
{
    // Устанавливаем политику контекстного меню
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested,
            this, &DynamicListWidget::showContextMenu);
}

void DynamicListWidget::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu;

    QAction *addAction = contextMenu.addAction("Добавить");
    QAction *editAction = contextMenu.addAction("Редактировать");
    QAction *deleteAction = contextMenu.addAction("Удалить");

    // Если нет выбранного элемента, отключаем Edit и Delete
    bool hasCurrentItem = (currentItem() != nullptr);
    editAction->setEnabled(hasCurrentItem);
    deleteAction->setEnabled(hasCurrentItem);

    // Подключаем действия к слотам
    connect(addAction, &QAction::triggered, this, &DynamicListWidget::onAddItem);
    connect(editAction, &QAction::triggered, this, &DynamicListWidget::onEditItem);
    connect(deleteAction, &QAction::triggered, this, &DynamicListWidget::onDeleteItem);

    // Показываем меню в позиции курсора
    contextMenu.exec(mapToGlobal(pos));
}

void DynamicListWidget::onAddItem()
{
    //bool ok;
    // QString text = QInputDialog::getText(this, tr("Добавление"),
    //                                      tr("Введите наименование элемента:"),
    //                                      QLineEdit::Normal,
    //                                      QString(), &ok);
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Добавление"));
    dialog.setLabelText(tr("Введите наименование элемента:"));
    dialog.setOkButtonText(tr("OK"));
    dialog.setCancelButtonText(tr("Отмена"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    bool ok = (dialog.exec() == QDialog::Accepted);
    QString text = dialog.textValue();
    if (ok && !text.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(text);
        addItem(item);
        // Делаем новый элемент текущим и выбранным
        setCurrentItem(item);
    } else if (ok && text.isEmpty()) {
        AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Наименование не может быть пустым"), 0, 3000);
    }
}

void DynamicListWidget::onEditItem()
{
    QListWidgetItem *item = currentItem();
    if (!item) return;

    bool ok;
    QString newText = QInputDialog::getText(this, tr("Редактирование"),
                                            tr("Редактирование элемента:"),
                                            QLineEdit::Normal,
                                            item->text(), &ok);
    if (ok && !newText.isEmpty()) {
        item->setText(newText);
    } else if (ok && newText.isEmpty()) {
        AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Наименование не может быть пустым"), 0, 3000);
    }
}

void DynamicListWidget::onDeleteItem()
{
    QListWidgetItem *item = currentItem();
    if (!item) return;

    // Удаляем элемент
    takeItem(this->row(item));
    delete item;
}
