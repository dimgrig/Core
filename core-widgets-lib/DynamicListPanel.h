#ifndef DYNAMICLISTPANEL_H
#define DYNAMICLISTPANEL_H

#include <QWidget>
#include "core-widgets-lib-global.h"

class QToolButton;
class QVBoxLayout;
class DynamicListWidget;

class CORE_WIDGETS_LIB_EXPORT DynamicListPanel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList items READ items WRITE setItems NOTIFY itemsChanged)

public:
    explicit DynamicListPanel(QWidget *parent = nullptr);

    QStringList items() const;
    void setItems(const QStringList &items);

    DynamicListWidget *listWidget() const { return m_list; }

    void clear();

signals:
    void itemsChanged();

private slots:
    void updateButtonsState();

private:
    void buildUi();

    DynamicListWidget *m_list = nullptr;
    QToolButton *m_addBtn = nullptr;
    QToolButton *m_editBtn = nullptr;
    QToolButton *m_removeBtn = nullptr;
};

#endif // DYNAMICLISTPANEL_H
