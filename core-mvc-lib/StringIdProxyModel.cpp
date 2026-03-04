#include "StringIdProxyModel.h"

StringIdProxyModel::StringIdProxyModel(int column, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_column(column)
{
    setDynamicSortFilter(true);

    setId("-1");
}

void StringIdProxyModel::setId(QString id)
{
    if (m_id != id) {
        m_id = id;
        m_useMultipleIds = false;
        invalidateFilter();
        emit idChanged(id);
    }
}

QString StringIdProxyModel::getId() const
{
    return m_id;
}

void StringIdProxyModel::setIds(const QSet<QString>& ids)
{
    if (m_ids != ids) {
        m_ids = ids;
        m_useMultipleIds = true;
        invalidateFilter();
    }
}

QSet<QString> StringIdProxyModel::getIds() const
{
    return m_ids;
}

bool StringIdProxyModel::filterAcceptsRow(int source_row,
                                                   const QModelIndex &source_parent) const
{
    if (!(m_column < sourceModel()->columnCount())) {
        return false;
    }

    // // Если фильтр не установлен, показываем все строки
    // if (m_uniqueId == "") {
    //     return true;
    // }

    // Получаем индекс столбца с ID
    QModelIndex index = sourceModel()->index(source_row, m_column, source_parent);

    if (!index.isValid()) {
        return false;
    }

    // Получаем ID из модели
    QString rowId = sourceModel()->data(index).toString();

    // Применяем фильтр
    if (m_useMultipleIds) {
        return m_ids.contains(rowId);
    } else {
        return (rowId == m_id) || rowId.isEmpty();
    }
}

void StringIdProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}
