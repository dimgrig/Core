#include "MenuCornerWidget.h"
#include "ui_menu_corner_widget.h"

#include "IconManager.h"

MenuCornerWidget::MenuCornerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::menu_corner_widget)
{
    ui->setupUi(this);

    this->set_pb_connection_visible(false);

    this->ui->le_auth_user->setDisabled(true);
    this->ui->l_logo->setPixmap(IconManager::applicationLogo());
    this->ui->l_logo->setVisible(false);

    this->ui->tb_logout->setIconSize(ICON_SIZE_24);
    this->ui->tb_logout->setIcon(ICON_LOGOUT);

    connect(this->ui->tb_logout, &QToolButton::clicked, this, &MenuCornerWidget::logout);
}

MenuCornerWidget::~MenuCornerWidget() {
    delete ui;
}

void MenuCornerWidget::set_auth_user(QString auth_user_str) {
    this->ui->le_auth_user->setText(auth_user_str);
}

void MenuCornerWidget::set_pb_connection_visible(bool visible) {
    this->ui->pb_connection->setVisible(visible);
}

ConnectionButton *MenuCornerWidget::get_pb_connection() {
    return this->ui->pb_connection;
}
