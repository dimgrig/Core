#ifndef DATETIMEUTCDELEGATE_H
#define DATETIMEUTCDELEGATE_H

#include <QStyledItemDelegate>
#include <QDateEdit>
#include <QPainter>

#include "CoreConfig.h"

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT DateTimeUTCDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit DateTimeUTCDelegate(QObject *parent = nullptr);

    QString get_format() const {return _format;};
    void set_format(QString format) {_format = format;};

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override ;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const;
private:
    QString _format = DATE_TIME_FORMAT_WITHOUT_MS;
};

#endif // DATETIMEUTCDELEGATE_H
