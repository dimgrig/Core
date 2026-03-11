#ifndef TEMPLATEEDITRELATIONALTABLEMODEL_H
#define TEMPLATEEDITRELATIONALTABLEMODEL_H

#include <memory>

#include "BaseSqlRelationalTableModel.h"

#include "core-mvc-lib-global.h"

template<class T>
class CORE_MVC_LIB_EXPORT TemplateEditRelationalTableModel : public BaseSqlRelationalTableModel {
public:
    explicit TemplateEditRelationalTableModel(QObject *parent = nullptr);

    void _clear();
    //void get_data(std::vector<T*> &data);
    const std::vector<std::shared_ptr<T>>& get_shared_data_ref() const;

    void load();
    bool save() override;
    void sync_v();
protected:
    // virtual void read_config() = 0;
    // virtual void write_config() = 0;

    QList<QList<QString>> modelData;
    std::vector<std::shared_ptr<T>> _v;
};

#include "TemplateEditRelationalTableModel.hpp"

#endif // TEMPLATEEDITRELATIONALTABLEMODEL_H
