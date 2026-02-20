#include "MenuBar.h"

#include "MenuCornerWidget.h"

MenuBar::MenuBar(QWidget *parent)
    : QMenuBar(parent)
{
    this->addMenu(filemenu);
    this->addMenu(settingsmenu);
    this->addMenu(viewmenu);
    this->addMenu(usermenu);
    this->addMenu(helpmenu);
}

void MenuBar::set_auth_user(QString auth_user_str) {
    auto cw = this->cornerWidget();
    auto mcw = dynamic_cast<MenuCornerWidget*>(cw);
    if (mcw != nullptr) {
        mcw->set_auth_user(auth_user_str);
    }
}
