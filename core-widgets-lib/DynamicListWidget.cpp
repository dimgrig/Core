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

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString text = dialog.textValue().trimmed();
    if (text.isEmpty()) {
        AQP::warning(this, tr("Внимание"), tr("Ошибка"),
                     tr("Наименование не может быть пустым"), 0, 3000);
        return;
    }

    auto *item = new QListWidgetItem(text);
    addItem(item);
    setCurrentItem(item);
    emit itemsChanged();
}

void DynamicListWidget::onEditItem()
{
    this->editItem(currentItem());
}

void DynamicListWidget::onDeleteItem()
{
    this->deleteItem(currentItem());
}

void DynamicListWidget::editItem(QListWidgetItem *item)
{
    if (!item) return;

    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Редактирование"));
    dialog.setLabelText(tr("Редактирование элемента:"));
    dialog.setOkButtonText(tr("OK"));
    dialog.setCancelButtonText(tr("Отмена"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    dialog.setTextValue(item->text());

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString newText = dialog.textValue().trimmed();
    if (newText.isEmpty()) {
        AQP::warning(this, tr("Внимание"), tr("Ошибка"),
                     tr("Наименование не может быть пустым"), 0, 3000);
        return;
    }

    // Ничего не поменялось — выходим без эмита
    if (newText == item->text())
        return;

    item->setText(newText);
    emit itemsChanged();
}

void DynamicListWidget::deleteItem(QListWidgetItem *item)
{
    if (!item) return;

    if (!AQP::question(this,
                  tr("Удаление"),
                  tr("Удалить элемент «%1»?").arg(item->text()),
                  QString(),                 // detailedText — пусто
                  tr("Да"),
                  tr("Нет")))
        return;

    // takeItem возвращает владение элементом; удаляем явно
    const int row = this->row(item);
    if (row < 0)
        return;

    delete takeItem(row); // takeItem возвращает QListWidgetItem*
    emit itemsChanged();
}
