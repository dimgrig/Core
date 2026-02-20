#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>
#include <QAction>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT MenuBar : public QMenuBar {
Q_OBJECT
public:
    MenuBar(QWidget *parent = nullptr);

    void set_auth_user(QString auth_user_str);

    QMenu *filemenu = new QMenu("&Файл", this);
    QMenu *settingsmenu = new QMenu("Настройки", this);
    QMenu *viewmenu = new QMenu("Вид", this);
    QMenu *usermenu = new QMenu("Пользователь", this);
    QMenu *helpmenu = new QMenu("Справка", this);
};

#endif // MENUBAR_H
