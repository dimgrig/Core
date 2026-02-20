#ifndef MENUCORNERWIDGET_H
#define MENUCORNERWIDGET_H

#include <QWidget>

#include "core-widgets-lib-global.h"

QT_BEGIN_NAMESPACE
namespace Ui { class menu_corner_widget; }
QT_END_NAMESPACE

class CORE_WIDGETS_LIB_EXPORT MenuCornerWidget : public QWidget {
    Q_OBJECT
public:
    MenuCornerWidget(QWidget *parent = nullptr);
    virtual ~MenuCornerWidget();

    void set_auth_user(QString auth_user_str);

signals:
    void logout();
public slots:

private slots:

private:
    Ui::menu_corner_widget *ui;


};

#endif // MENUCORNERWIDGET_H
