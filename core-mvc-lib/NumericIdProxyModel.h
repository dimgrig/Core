#ifndef NUMERICIDPROXYMODEL_H
#define NUMERICIDPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QSet>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT NumericIdProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int id READ getId WRITE setId NOTIFY idChanged)

public:
    explicit NumericIdProxyModel(int column = 0, QObject *parent = nullptr);

    // Установка ID для фильтрации
    void setId(int id);
    int getId() const;

    // Можно фильтровать по нескольким ID
    void setIds(const QSet<int>& ids);
    QSet<int> getIds() const;

    // Метод для принудительного обновления фильтра
    void invalidateFilter();
signals:
    void idChanged(int id);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_column = 0;
    int m_id = -1;
    QSet<int> m_ids;    // Если нужна фильтрация по нескольким устройствам
    bool m_useMultipleIds = false;
};

#endif // NUMERICIDPROXYMODEL_H
