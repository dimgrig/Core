#include "BaseEditTableModel.h"

BaseEditTableModel::BaseEditTableModel(QWidget *parent) :
    BaseTableModel(parent)
{

}

Qt::ItemFlags BaseEditTableModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

Qt::DropActions BaseEditTableModel::supportedDropActions() const {
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList BaseEditTableModel::mimeTypes() const {
    QStringList types;
    types << "application/x-qabstractitemmodeldatalist";
    return types;
}

QMimeData *BaseEditTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    if (indexes.empty()) {
        return mimeData;
    }

    //qCDebug(logDebug()) << indexes.first().row() << indexes.last().row();

    //foreach (const QModelIndex &index, indexes) {
    //for (auto it = indexes.begin(); it < indexes.end(); ++it) {
    for (int row = indexes.first().row(); row <= indexes.last().row(); ++row) {
        for (int col = 0; col < this->columnCount(); ++col) {
            auto index = this->index(row, col);
            if (index.isValid()) {
                QString text = data(index, Qt::DisplayRole).toString();
                stream << text;
            }
        }
    }

    mimeData->setData("application/x-qabstractitemmodeldatalist", encodedData);
    return mimeData;
}

bool BaseEditTableModel::canDropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(parent);

    if (!data->hasFormat("application/x-qabstractitemmodeldatalist"))
        return false;

    if (column > 0)
        return false;

    return true;
}

bool BaseEditTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd()) {
         QString text;
         stream >> text;
         newItems << text;
         if (newItems.size() % this->columnCount() == 0) {
            ++rows;
         }
         //qCDebug(logDebug()) << newItems.size() << this->columnCount();
     }

    insertRows(beginRow, rows, QModelIndex());
    row = beginRow;
    int col = 0;
    int i = 0;
    foreach (const QString &text, newItems) {
        QModelIndex idx = index(row, col, QModelIndex());
        setData(idx, text);
        ++i;
        if (i % this->columnCount() == 0) {
            ++row;
            col = 0;
        } else {
            ++col;
        }
    }

    emit view_repaint();

    return true;
}
