#ifndef VALUELISTDELEGATE_H
#define VALUELISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "core-mvc-lib-global.h"

CORE_MVC_LIB_EXPORT QList<double> str_parse_to_list(const QString &value_str);
CORE_MVC_LIB_EXPORT QString join_list_to_string(const QList<double> &values);

class CORE_MVC_LIB_EXPORT ValueListDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ValueListDialog(QStringList header_sl, QWidget *parent = nullptr);

    void setValues(const QList<double> &values);
    QList<double> getValues() const;

private slots:
    void addItem();
    void removeItem();

private:
    QTableWidget *tableWidget;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

class ValueListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ValueListDelegate(QObject *parent = nullptr);

    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private:

};

#endif // VALUELISTDELEGATE_H
