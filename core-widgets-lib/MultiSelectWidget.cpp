#include "MultiSelectWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QToolButton>
#include <QFrame>
#include <QWidgetAction>

#include "IconManager.h"

MultiSelectWidget::MultiSelectWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);

    // Кнопка "+"
    _addButton = new QPushButton("", this);
    _addButton->setStyleSheet("QPushButton { font-weight: bold; font-size: 16px; }");
    _addButton->setIcon(ICON_PLUS);
    _addButton->setIconSize(ICON_SIZE_24);
    mainLayout->addWidget(_addButton);

    _flowContainer = new QWidget(this);
    _flowLayout = new FlowLayout(_flowContainer, 2, 2, 2); // отступы
    _flowContainer->setLayout(_flowLayout);
    _flowContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(_flowContainer);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // Меню с поиском
    _popupMenu = new QMenu(this);
    auto *menuWidget = new QWidget();  // без родителя, чтобы избежать двойного удаления
    menuWidget->setMinimumWidth(250);
    auto *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setContentsMargins(4, 4, 4, 4);

    _filterEdit = new QLineEdit(menuWidget);
    _filterEdit->setPlaceholderText(tr("Поиск..."));
    menuLayout->addWidget(_filterEdit);

    _channelListWidget = new QListWidget(menuWidget);
    _channelListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    _channelListWidget->setMaximumHeight(300);
    menuLayout->addWidget(_channelListWidget);

    auto *widgetAction = new QWidgetAction(_popupMenu);
    widgetAction->setDefaultWidget(menuWidget);
    _popupMenu->addAction(widgetAction);

    connect(_addButton, &QPushButton::clicked, this, &MultiSelectWidget::onAddButtonClicked);
    connect(_filterEdit, &QLineEdit::textChanged, this, &MultiSelectWidget::onFilterTextChanged);
    connect(_channelListWidget, &QListWidget::itemChanged,
            this, &MultiSelectWidget::onMenuItemToggled);
}

void MultiSelectWidget::setItems(const QMap<int, QString> &items) {
    _availableItems = items;
    // Удаляем из выбора те id, которых больше нет в новом наборе
    QSet<int> validIds;
    for (auto it = items.begin(); it != items.end(); ++it)
        validIds.insert(it.key());
    _selectedIds.intersect(validIds);
    rebuildChips();
    emit selectionChanged(selectedIds());
}

QVector<int> MultiSelectWidget::selectedIds() const {
    return _selectedIds.values().toVector();
}

void MultiSelectWidget::setSelectedIds(const QVector<int> &ids) {
    _selectedIds = QSet<int>(ids.begin(), ids.end());
    // Оставляем только те, что есть среди availableItems
    _selectedIds.intersect(QSet<int>(_availableItems.keyBegin(), _availableItems.keyEnd()));
    rebuildChips();
    emit selectionChanged(selectedIds());
}

void MultiSelectWidget::onAddButtonClicked() {
    populateMenu();
    _popupMenu->popup(_addButton->mapToGlobal(QPoint(0, _addButton->height())));
    _filterEdit->setFocus();
}

void MultiSelectWidget::populateMenu() {
    _channelListWidget->blockSignals(true);
    _channelListWidget->clear();

    const QString filterText = _filterEdit->text().trimmed().toLower();
    for (auto it = _availableItems.constBegin(); it != _availableItems.constEnd(); ++it) {
        if (!filterText.isEmpty() && !it.value().toLower().contains(filterText))
            continue;

        auto *item = new QListWidgetItem(it.value());
        item->setData(Qt::UserRole, it.key());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(_selectedIds.contains(it.key()) ? Qt::Checked : Qt::Unchecked);
        _channelListWidget->addItem(item);
    }
    _channelListWidget->blockSignals(false);
}

void MultiSelectWidget::onFilterTextChanged(const QString & /*text*/) {
    populateMenu();
}

void MultiSelectWidget::onMenuItemToggled(QListWidgetItem *item) {
    int id = item->data(Qt::UserRole).toInt();
    if (item->checkState() == Qt::Checked)
        _selectedIds.insert(id);
    else
        _selectedIds.remove(id);

    rebuildChips();
    emit selectionChanged(selectedIds());
}

void MultiSelectWidget::rebuildChips() {
    // Удаляем старые виджеты из FlowLayout
    QLayoutItem *child;
    while ((child = _flowLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = child->widget())
            delete w;
        delete child;
    }

    // Создаём чипсы для выбранных элементов
    for (int id : _selectedIds) {
        if (_availableItems.contains(id)) {
            QWidget *chip = createChip(id, _availableItems[id]);
            _flowLayout->addWidget(chip);
        }
    }

    // Добавляем растягивающийся элемент, чтобы чипсы прижимались влево
    _flowLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    // Обновляем геометрию контейнера и всего виджета
    _flowContainer->adjustSize();
    adjustSize();
    updateGeometry(); // сигнализируем родительскому layout об изменении размера
}

QWidget* MultiSelectWidget::createChip(int id, const QString &name) {
    auto *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    //frame->setFixedHeight(24);
    frame->setStyleSheet("QFrame { background: palette(button); border-radius: 4px; }");

    auto *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(4, 0, 2, 0);
    layout->setSpacing(2);

    auto *label = new QLabel(name, frame);
    layout->addWidget(label);

    auto *closeBtn = new QToolButton(frame);
    closeBtn->setStyleSheet("QToolButton { font-weight: bold; }");
    closeBtn->setIcon(ICON_BASKET);
    closeBtn->setIconSize(ICON_SIZE_16);
    layout->addWidget(closeBtn);

    connect(closeBtn, &QToolButton::clicked, this, [this, id]() {
        onRemoveChip(id);
    });

    return frame;
}

void MultiSelectWidget::onRemoveChip(int id) {
    _selectedIds.remove(id);
    rebuildChips();
    emit selectionChanged(selectedIds());
}
