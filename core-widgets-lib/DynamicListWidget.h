#ifndef DYNAMICLISTWIDGET_H
#define DYNAMICLISTWIDGET_H

#include <QListWidget>
#include <QPoint>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT DynamicListWidget : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList items READ getItems WRITE setItems)

public:
    explicit DynamicListWidget(QWidget *parent = nullptr);

    QStringList getItems() const;
    void setItems(const QStringList &items);
protected:
    void showContextMenu(const QPoint &pos);

signals:
    void itemsChanged();
public slots:
    void onAddItem();
    void onEditItem();
    void onDeleteItem();

    void editItem(QListWidgetItem *item);
    void deleteItem(QListWidgetItem *item);

private:
    void setupContextMenu();
};

#endif // DYNAMICLISTWIDGET_H
