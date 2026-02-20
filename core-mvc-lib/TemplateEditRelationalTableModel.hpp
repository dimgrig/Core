#include "TemplateEditRelationalTableModel.h"

#include <QWidget>
#include <QSqlError>
#include <QStandardItemModel>

#include "Logger.h"
#include "templates.h"
#include "Profiler.h"

template<class T>
TemplateEditRelationalTableModel<T>::TemplateEditRelationalTableModel(QObject *parent) :
    BaseSqlRelationalTableModel(parent)
{

}

template<class T>
void TemplateEditRelationalTableModel<T>::_clear() {
    //QStandardItemModel::clear();
    //    for (int i = this->rowCount() - 1; i >= 0; --i) {
    //        this->removeRow(i);
    //    }
    this->removeRows(0, this->rowCount());
    //this->clear(); //clear hidden columns
    //this->set_filename("");
    this->_v.clear();
    //initialize(); //clear hidden columns
}

// template<class T>
// void TemplateEditRelationalTableModel<T>::get_data(std::vector<T*> &data_v) {
//     //ATMLReader::TestDescription(this->filename().toStdString(), tests);
//     data_v = _v;
// }

// Метод, возвращающий константную ссылку (без копирования)
template<class T>
const std::vector<std::shared_ptr<T>>& TemplateEditRelationalTableModel<T>::get_shared_data_ref() const {
    return _v; // Возвращает ссылку на внутренний вектор
}

template<class T>
void TemplateEditRelationalTableModel<T>::load() {
    this->sync_v();

    emit view_repaint();
}

template<class T>
void TemplateEditRelationalTableModel<T>::save() {
    this->database().transaction();
    if (this->submitAll()) {
        this->database().commit();
        this->select(); // Важно: обновляем данные модели
        this->set_modified(false);
        this->sync_v();
        qCDebug(logDebug()) << "Изменения успешно сохранены!";

        emit saved();
    } else {
        this->database().rollback();
        qCDebug(logDebug()) << "Ошибка, не удалось сохранить изменения: " + this->lastError().text();
    }

    //this->write_config();
}

template<class T>
void TemplateEditRelationalTableModel<T>::sync_v() {
    this->_v.clear();

    QStringList sl;
    for (int row = 0; row < rowCount(); ++row) {
        sl.clear();
        for (int col = 0; col < columnCount(); ++col) {
            //sl << item(row, col)->text();
            sl << this->data(this->index(row, col)).toString();
        }
        auto t = std::make_shared<T>(nullptr); //this->get_parent()
        t->create_from_model(sl);
        _v.push_back(t);
    }
}

