#ifndef TEMPLATEEDITTABLEMODEL_H
#define TEMPLATEEDITTABLEMODEL_H

#include "BaseEditTableModel.h"

template<class T>
class TemplateEditTableModel : public BaseEditTableModel {
public:
    explicit TemplateEditTableModel(QWidget *parent = nullptr);

    void _clear();
    //void get_data(std::vector<T*> &data);
    const std::vector<std::shared_ptr<T>>& get_shared_data_ref() const;

    void load();
    void save();
protected:
    virtual void read_config() = 0;
    virtual void write_config() = 0;

    QList<QList<QString>> modelData;
    std::vector<std::shared_ptr<T>> _v;
};

#include "TemplateEditTableModel.hpp"

#endif // TEMPLATEEDITTABLEMODEL_H
