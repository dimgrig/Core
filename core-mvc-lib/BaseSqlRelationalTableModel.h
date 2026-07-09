#ifndef BASESQLRELATIONALTABLEMODEL_H
#define BASESQLRELATIONALTABLEMODEL_H

#include <set>

#include <QSet>
#include <QSqlRelationalTableModel>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT BaseSqlRelationalTableModel : public QSqlRelationalTableModel {
    Q_OBJECT
public:
    explicit BaseSqlRelationalTableModel(QObject *parent= nullptr);

    QObject * get_parent() const {return _parent;};

    virtual bool save();

    virtual bool setVirtualData(int row, int virtualColumnIndex, const QVariant& value);
    virtual QVariant virtualData(int row, int virtualColumnIndex, int role = Qt::DisplayRole) const;
    virtual int findRowByKey(const QVariant& keyValue, const int column = 0) const;

    bool isVirtualColumn(int column) const;
    int virtualColumnIndex(int column) const;
    int virtualColumnAbsoluteIndex(int virtualColumnIndex) const;

    bool is_modified() const {return _is_modified;};
    void set_modified(bool is_modified = false);

    bool is_editable() const {return _is_editable;};
    void set_editable(bool is_editable = false);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    int findRowById(int id, int column = 0) const;

    void setRelation(int column, const QSqlRelation &relation) override;
    void updateRelations();
    void updateRelations(int column);
    QString buildTimestampCondition(const QString& fieldName, const QString& filterExpr);

    //bool cleanUpOldEntries(const QString& table, const QString& timestamp, int keepDays);
    static int cleanUpOldEntriesAsync(const QString& table, const QString& timestampField,
                                       int keepDays,
                                       std::function<void(int,int)> progressCallback = nullptr,
                                       bool* cancelFlag = nullptr);

    QString getFullFilter() const {return _fullFilter;};
    QString getTimestampOp() const {return _timestampOp;};
    qint64 getStartSecs() const {return _startSecs;};
    qint64 getEndSecs() const {return _endSecs;};
signals:
    void view_repaint();
    void view_repaint(int row, int col);

    void modified(bool modified);
    void saved();

    void filterChanged(const QString fullFilter);
public slots:
    void setRawFilterColumn(int column);
    void filter_changed(int column, const QString& text);
private slots:
    void data_changed(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void mark_as_modified();
protected:

private:
    QObject *_parent = nullptr;

    bool _is_modified = false;
    bool _is_editable = true;

    std::set<int> _relation_column_s = {};

    QMap<int, QString> _filters;
    QString _fullFilter;
    QSet<int> _rawFilterColumns;

    QString _timestampOp;
    qint64 _startSecs = 0;
    qint64 _endSecs = 0;
};

#endif // BASESQLRELATIONALTABLEMODEL_H
