#ifndef NUMERICIDPROXYMODEL_H
#define NUMERICIDPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QSet>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT NumericIdProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(long long id READ getId WRITE setId NOTIFY idChanged)

public:
    explicit NumericIdProxyModel(int column = 0, QObject *parent = nullptr);

    // Установка ID для фильтрации
    void setId(long long id);
    long long getId() const;

    // Можно фильтровать по нескольким ID
    void setIds(const QSet<long long>& ids);
    QSet<long long> getIds() const;

    // Метод для принудительного обновления фильтра
    void invalidateFilter();
signals:
    void idChanged(long long id);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    int m_column = 0;
    long long m_id = -1;
    QSet<long long> m_ids;    // Если нужна фильтрация по нескольким устройствам
    bool m_useMultipleIds = false;
};

#endif // NUMERICIDPROXYMODEL_H
