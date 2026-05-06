#include "DetailListWidget.h"

#include <QVBoxLayout>
#include <QItemSelectionModel>

DetailListWidget::DetailListWidget(QString name, QWidget *parent)
    : QWidget(parent)
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(name);

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
    //layout->setContentsMargins(0, 0, 0, 0);
}

void DetailListWidget::setModel(QAbstractItemModel *model, DetailListWidgetPanelFactory factory, int idColumn, int displayColumn)
{
    m_model = model;
    m_factory = factory;
    m_idColumn = idColumn;
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

    // if (row == 0) {
    //     QModelIndex firstIndex = m_model->index(0, m_displayColumn);
    //     m_listView->selectionModel()->select(firstIndex, QItemSelectionModel::ClearAndSelect);
    // }
}

void DetailListWidget::removePanel(int row)
{
    if (row >= 0 && row < m_stack->count()) {
        QWidget *w = m_stack->widget(row);
        m_stack->removeWidget(w);
        //w->deleteLater();
    }
}

void DetailListWidget::clearPanels()
{
    while (m_stack->count() > 0) {
        QWidget *w = m_stack->widget(0);
        m_stack->removeWidget(w);
        //w->deleteLater();
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

void DetailListWidget::setCurrentRow(int row)
{
    if (!m_model || row < 0 || row >= m_model->rowCount())
        return;

    QModelIndex index = m_model->index(row, m_displayColumn);
    if (index.isValid()) {
        m_listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        m_listView->scrollTo(index); // опционально: прокрутить, если список длинный
        // Это автоматически вызовет onSelectionChanged() и переключит панель
    }
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
        emit currentRowIdChanged(currentId());
    }
}

int DetailListWidget::currentId() const
{
    if (!m_model || m_stack->currentIndex() < 0)
        return -1;                         // или другое значение "нет данных"

    int row = m_stack->currentIndex();
    QModelIndex idx = m_model->index(row, m_idColumn);
    if (!idx.isValid())
        return -1;

    return m_model->data(idx, Qt::DisplayRole).toInt();  // предполагаем, что ID – число
}

void DetailListWidget::updatePanels()
{
    if (!m_model || !m_factory)
        return;

    // Запоминаем текущую выбранную строку (если есть)
    int previousRow = m_stack->currentIndex();

    // Очищаем старые панели
    clearPanels();

    // Создаём новые по текущим строкам модели
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QModelIndex index = m_model->index(row, m_idColumn); // любой валидный индекс
        QWidget *panel = m_factory(row, index);
        if (panel) {
            m_stack->insertWidget(row, panel);
        }
    }

    // Пытаемся восстановить выделение на ту же позицию, если возможно
    if (previousRow >= 0 && previousRow < m_stack->count()) {
        m_stack->setCurrentIndex(previousRow);
        // и выделить в списке
        QModelIndex idx = m_model->index(previousRow, m_displayColumn);
        if (idx.isValid())
            m_listView->setCurrentIndex(idx);
    } else if (m_stack->count() > 0) {
        // иначе выделяем первую
        QModelIndex idx = m_model->index(0, m_displayColumn);
        if (idx.isValid())
            m_listView->setCurrentIndex(idx);
    }
}
