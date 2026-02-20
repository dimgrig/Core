#ifndef BASETABLEMODEL_H
#define BASETABLEMODEL_H

#include <QWidget>
#include <QStandardItemModel>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT BaseTableModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit BaseTableModel(QWidget *parent= nullptr);

    QWidget * get_parent() const {return _parent;};

    QString filename() const { return _filename; }
    void set_filename(QString filename) {_filename = filename;};

    void set_checked_filename(QString filename);

    bool is_modified() const {return _is_modified;};
    void set_modified(bool is_modified = false);
signals:
    void view_repaint();
    void view_repaint(int row, int col);

    void modified(bool modified);
    //void calculate_statistic();
private slots:
    void data_changed(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void mark_as_modified();
private:
    QWidget *_parent;

    QString _filename;

    bool _is_modified = false;
};


#endif // BASETABLEMODEL_H
