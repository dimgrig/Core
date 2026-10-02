#include "DynamicListPanel.h"
#include "DynamicListWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QMessageBox>

#include "IconManager.h"

DynamicListPanel::DynamicListPanel(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
}

void DynamicListPanel::buildUi()
{
    m_list = new DynamicListWidget(this);

    m_addBtn    = new QToolButton(this);
    m_editBtn   = new QToolButton(this);
    m_removeBtn = new QToolButton(this);

    m_addBtn->setIcon(ICON_ADD);
    m_editBtn->setIcon(ICON_SETTINGS);
    m_removeBtn->setIcon(ICON_DELETE);

    m_addBtn->setIconSize(ICON_SIZE_32);
    m_editBtn->setIconSize(ICON_SIZE_32);
    m_removeBtn->setIconSize(ICON_SIZE_32);

    m_addBtn->setToolTip(tr("Добавить"));
    m_editBtn->setToolTip(tr("Редактировать"));
    m_removeBtn->setToolTip(tr("Удалить"));

    m_addBtn->setAutoRaise(true);
    m_editBtn->setAutoRaise(true);
    m_removeBtn->setAutoRaise(true);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(2);
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    mainLayout->addWidget(m_list, 1);
    mainLayout->addLayout(btnLayout);

    // Кнопки дёргают те же слоты, что и контекстное меню
    connect(m_addBtn,    &QToolButton::clicked, m_list, &DynamicListWidget::onAddItem);
    connect(m_editBtn,   &QToolButton::clicked, m_list, &DynamicListWidget::onEditItem);
    connect(m_removeBtn, &QToolButton::clicked, m_list, &DynamicListWidget::onDeleteItem);

    // Синхронизация состояния кнопок с состоянием списка
    connect(m_list, &QListWidget::currentItemChanged,
            this,   &DynamicListPanel::updateButtonsState);
    connect(m_list, &QListWidget::itemDoubleClicked,
            this,   &DynamicListPanel::updateButtonsState);
    connect(m_list, &DynamicListWidget::itemsChanged, // если добавите сигнал
            this,   &DynamicListPanel::updateButtonsState);

    // Реакция на добавление/удаление элементов
    // (QListWidget не шлёт сигнал при addItem/takeItem — можно подписаться
    //  на model()->rowsInserted / rowsRemoved)
    connect(m_list->model(), &QAbstractItemModel::rowsInserted,
            this, &DynamicListPanel::updateButtonsState);
    connect(m_list->model(), &QAbstractItemModel::rowsRemoved,
            this, &DynamicListPanel::updateButtonsState);

    updateButtonsState();
}

void DynamicListPanel::updateButtonsState()
{
    const bool hasCurrent = m_list->currentItem() != nullptr;
    m_editBtn->setEnabled(hasCurrent);
    m_removeBtn->setEnabled(hasCurrent);
}

QStringList DynamicListPanel::items() const
{
    return m_list->getItems();
}

void DynamicListPanel::setItems(const QStringList &items)
{
    m_list->setItems(items);
    updateButtonsState();
    emit itemsChanged();
}

void DynamicListPanel::clear() {
    m_list->clear();
}
