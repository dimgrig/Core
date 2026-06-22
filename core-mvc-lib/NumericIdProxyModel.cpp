#include "NumericIdProxyModel.h"

NumericIdProxyModel::NumericIdProxyModel(int column, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_column(column)
{
    setDynamicSortFilter(true);

    setId(-1);
}

void NumericIdProxyModel::setId(long long id)
{
    if (m_id != id) {
        m_id = id;
        m_useMultipleIds = false;
        invalidateFilter();
        emit idChanged(id);
    }
}

long long NumericIdProxyModel::getId() const
{
    return m_id;
}

void NumericIdProxyModel::setIds(const QSet<long long>& ids)
{
    if (m_ids != ids) {
        m_ids = ids;
        m_useMultipleIds = true;
        invalidateFilter();
    }
}

QSet<long long> NumericIdProxyModel::getIds() const
{
    return m_ids;
}

bool NumericIdProxyModel::filterAcceptsRow(int source_row,
                                           const QModelIndex &source_parent) const
{
    if (!(m_column < sourceModel()->columnCount())) {
        return false;
    }

    // Если фильтр не установлен, показываем все строки
//    if (!m_useMultipleIds && m_id == -1) {
//        return true;
//    }

    // Получаем индекс столбца с ID
    QModelIndex index = sourceModel()->index(source_row, m_column, source_parent);

    if (!index.isValid()) {
        return false;
    }

    // Получаем ID из модели
    bool ok;
    long long rowId = sourceModel()->data(index).toLongLong(&ok);

    if (!ok) {
        // Если данные не могут быть преобразованы в int, пробуем получить через роль
        QVariant data = sourceModel()->data(index, Qt::UserRole);
        rowId = data.toLongLong();
    }

    // Применяем фильтр
    if (m_useMultipleIds) {
        return m_ids.contains(rowId);
    } else {
        return rowId == m_id || rowId == 0;
    }
}

QVariant NumericIdProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        // section – это индекс строки в прокси-модели (отфильтрованной)
        return section + 1; // нумерация с 1
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

void NumericIdProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}
