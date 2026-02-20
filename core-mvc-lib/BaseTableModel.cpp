#include "BaseTableModel.h"

#include "aqp.h"

BaseTableModel::BaseTableModel(QWidget *parent)
    : QStandardItemModel(parent)
{
    _parent = parent;

    connect(this, &QAbstractTableModel::dataChanged, this, &BaseTableModel::data_changed);
    connect(this, &QAbstractTableModel::rowsInserted, this, &BaseTableModel::mark_as_modified);
    connect(this, &QAbstractTableModel::rowsRemoved, this, &BaseTableModel::mark_as_modified);
}

void BaseTableModel::set_checked_filename(QString filename) {
    if (!filename.isEmpty())
        this->set_filename(filename);
    if (this->filename().isEmpty())
        throw AQP::Error(tr("no filename specified"));
}

void BaseTableModel::set_modified(bool is_modified) {
    _is_modified = is_modified;
    emit modified(_is_modified);
}

void BaseTableModel::data_changed(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    _is_modified = true;
    emit modified(_is_modified);
}

void BaseTableModel::mark_as_modified() {
    _is_modified = true;
    emit modified(_is_modified);
}
