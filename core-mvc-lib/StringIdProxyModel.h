#ifndef STRINGIDPROXYMODEL_H
#define STRINGIDPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QSet>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT StringIdProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString id READ getId WRITE setId NOTIFY idChanged)

public:
    explicit StringIdProxyModel(int column = 0, QObject *parent = nullptr);

    // Установка ID для фильтрации
    void setId(QString id);
    QString getId() const;

    // Можно фильтровать по нескольким ID
    void setIds(const QSet<QString>& ids);
    QSet<QString> getIds() const;

    // Метод для принудительного обновления фильтра
    void invalidateFilter();
signals:
    void idChanged(QString id);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
    int m_column = 0;
    QString m_id = "-1";
    QSet<QString> m_ids;    // Если нужна фильтрация по нескольким устройствам
    bool m_useMultipleIds = false;
};

#endif // STRINGIDPROXYMODEL_H
