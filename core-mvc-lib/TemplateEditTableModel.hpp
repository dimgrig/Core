#include "TemplateEditTableModel.h"

#include <core-lib/templates.h>
#include <core-lib/Profiler.h>

template<class T>
TemplateEditTableModel<T>::TemplateEditTableModel(QWidget *parent) :
    BaseEditTableModel(parent)
{

}


template<class T>
void TemplateEditTableModel<T>::_clear() {
    //QStandardItemModel::clear();
    //    for (int i = this->rowCount() - 1; i >= 0; --i) {
    //        this->removeRow(i);
    //    }
    this->removeRows(0, this->rowCount());
    //this->clear(); //clear hidden columns
    this->set_filename("");
    this->_v.clear();
    //initialize(); //clear hidden columns
}

// template<class T>
// void TemplateEditTableModel<T>::get_data(std::vector<T*> &data_v) {
//     //ATMLReader::TestDescription(this->filename().toStdString(), tests);
//     data_v = _v;
// }

// Метод, возвращающий константную ссылку (без копирования)
template<class T>
const std::vector<std::shared_ptr<T>>& TemplateEditTableModel<T>::get_shared_data_ref() const {
    return _v; // Возвращает ссылку на внутренний вектор
}

template<class T>
void TemplateEditTableModel<T>::load() {
    this->read_config();

    QMap<quint16, QList<QStandardItem*>> items_for_test;
    {
        Profiler p2("TestModel items_for_test");
        int index = 0;
        for (const auto &t : this->_v) {
            QList<QStandardItem*> items;
            QStringList sl = t->get_sl();
            for (const auto &element : sl) {
                items << new QStandardItem(element);
            }
            items_for_test[index++] = items; //sensor->get_data().sensor_id
        }
    }

    {
        Profiler p3("TestModel appendRow");
        QMapIterator<quint16, QList<QStandardItem*> > i(items_for_test);
        while (i.hasNext())
            appendRow(i.next().value());
    }

    {
        Profiler p4("TestModel view_repaint");
        emit view_repaint();
    }
}

template<class T>
void TemplateEditTableModel<T>::save() {
    this->_v.clear();

    QStringList sl;
    for (int row = 0; row < rowCount(); ++row) {
        sl.clear();
        for (int col = 0; col < columnCount(); ++col) {
            sl << item(row, col)->text();
        }
        auto t = std::make_shared<T>(this->get_parent());
        t->create_from_model(sl);
        _v.push_back(t);
    }

    this->write_config();
}

