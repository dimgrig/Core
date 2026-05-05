#include "DetailListWidget.h"

#include <QVBoxLayout>
#include <QItemSelectionModel>

DetailListWidget::DetailListWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_listView = new QListView(this);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_stack = new QStackedWidget(this);

    splitter->addWidget(m_listView);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3); // правая часть шире

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);
}

void DetailListWidget::setModel(QAbstractItemModel *model, int displayColumn)
{
    m_model = model;
    m_displayColumn = displayColumn;
    m_listView->setModel(model);
    m_listView->setModelColumn(displayColumn);

    // Очищаем старые панели, т.к. модель поменялась
    clearPanels();

    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &DetailListWidget::onSelectionChanged);
}

void DetailListWidget::insertPanel(int row, QWidget *panel)
{
    // Вставляем на нужную позицию; если row больше текущего количества,
    // QStackedWidget добавит в конец, что допустимо, но для строгого соответствия
    // лучше поддерживать размер равным количеству строк модели.
    //while (m_stack->count() <= row)
    //    m_stack->addWidget(new QWidget()); // заглушка, если нужно
    m_stack->insertWidget(row, panel);
}

void DetailListWidget::removePanel(int row)
{
    if (row >= 0 && row < m_stack->count()) {
        QWidget *w = m_stack->widget(row);
        m_stack->removeWidget(w);
        w->deleteLater();
    }
}

void DetailListWidget::clearPanels()
{
    while (m_stack->count() > 0) {
        QWidget *w = m_stack->widget(0);
        m_stack->removeWidget(w);
        w->deleteLater();
    }
}

QWidget *DetailListWidget::currentPanel() const
{
    return m_stack->currentWidget();
}

int DetailListWidget::currentRow() const
{
    return m_stack->currentIndex();
}

void DetailListWidget::onSelectionChanged()
{
    QModelIndexList indexes = m_listView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        return;
    }
    int row = indexes.first().row();
    if (row >= 0 && row < m_stack->count()) {
        m_stack->setCurrentIndex(row);
        emit currentRowChanged(row);
    }
}
