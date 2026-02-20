#ifndef BASEEDITTABLEMODEL_H
#define BASEEDITTABLEMODEL_H

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeData>

#include "BaseTableModel.h"

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT BaseEditTableModel : public BaseTableModel {
    Q_OBJECT
public:
    explicit BaseEditTableModel(QWidget *parent = nullptr);

    //Allowed operations, plus drag drop
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    //allow move
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    //Information carried when dragging
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    //Process the drop according to the information carried by drag
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
public slots:
    //void on_read_test_result(Test_c test);
protected:

};


#endif // BASEEDITTABLEMODEL_H
