#include "Auth.h"
#include "ui_Auth.h"

Auth::Auth(IUserRoleManager* roleManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Auth),
    _roleManager(roleManager),
    _users(new Users(roleManager, UserDefaultFilename, this))
{
    ui->setupUi(this);

    this->setWindowTitle("Авторизация");
    this->setModal(true);
    this->setWindowFlags(Qt::Dialog); //Qt::Dialog | Qt::SplashScreen

    this->ui->le_name->setFocus();

    this->ui->le_password->setEchoMode(QLineEdit::Password);

    connect(this->ui->pb_enter, &QPushButton::clicked, this, &Auth::accept);
    connect(this->ui->pb_cancel, &QPushButton::clicked, this, &Auth::reject);

    connect(this->_users, &Users::user_fail, this, &Auth::user_fail);
    connect(this->_users, &Users::password_fail, this, &Auth::password_fail);
}

Auth::~Auth() {
    delete ui;
}

void Auth::accept() {
    this->ui->l_text->setText("");    
    this->ui->le_name->setProperty("fail", QVariant::fromValue(false));
    this->ui->le_password->setProperty("fail", QVariant::fromValue(false));
    this->ui->le_name->setStyleSheet(this->ui->le_name->styleSheet());
    this->ui->le_password->setStyleSheet(this->ui->le_password->styleSheet());

    std::string login = this->ui->le_name->text().toStdString();
    std::string password = this->ui->le_password->text().toStdString();

    User_s* user = _users->find(login, password);
    if (nullptr != user) {
        auth_user.user = *user;
        auth_user.setRoleManager(_roleManager);
        emit auth_ok();
        this->ui->le_name->clear();
        this->ui->le_password->clear();
    } else {
        emit auth_failed();
    }    
}

void Auth::reject() {
    this->close();
    QCoreApplication::quit();
}

void Auth::users_show() {
    this->_users->show();
}

void Auth::enter(std::string login, std::string password) {
    this->ui->le_name->setText(login.c_str());
    this->ui->le_password->setText(password.c_str());
    //this->enter();
}

void Auth::user_fail() {
    this->ui->l_text->setText("Пользователь не найден");
    this->ui->le_password->clear();
    this->ui->le_name->setProperty("fail", QVariant::fromValue(true));
    this->ui->le_name->setStyleSheet(this->ui->le_name->styleSheet());
    this->on_shake();
}

void Auth::password_fail() {
    this->ui->l_text->setText("Неверный пароль");
    this->ui->le_password->clear();
    this->ui->le_password->setProperty("fail", QVariant::fromValue(true));
    this->ui->le_password->setStyleSheet(this->ui->le_password->styleSheet());
    this->on_shake();
}

void Auth::on_shake() {
    QPropertyAnimation *pAnimation = new QPropertyAnimation(this, "pos");
    pAnimation->setDuration(200);
    pAnimation->setLoopCount(1);
    double value = 3;
    pAnimation->setKeyValueAt(0, QPoint(geometry().x() - value/2, geometry().y() - value/2));
    pAnimation->setKeyValueAt(0.1, QPoint(geometry().x() + value, geometry().y() + value));
    pAnimation->setKeyValueAt(0.2, QPoint(geometry().x() - value, geometry().y() + value));
    pAnimation->setKeyValueAt(0.3, QPoint(geometry().x() + value, geometry().y() - value));
    pAnimation->setKeyValueAt(0.4, QPoint(geometry().x() - value, geometry().y() - value));
    pAnimation->setKeyValueAt(0.5, QPoint(geometry().x() + value, geometry().y() + value));
    pAnimation->setKeyValueAt(0.6, QPoint(geometry().x() - value, geometry().y() + value));
    pAnimation->setKeyValueAt(0.7, QPoint(geometry().x() + value, geometry().y() - value));
    pAnimation->setKeyValueAt(0.8, QPoint(geometry().x() - value, geometry().y() - value));
    pAnimation->setKeyValueAt(0.9, QPoint(geometry().x() + value, geometry().y() + value));
    pAnimation->setKeyValueAt(1, QPoint(geometry().x() - value/2, geometry().y() - value/2));
    pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Auth::exit() {
    User_s blanc_user;
    auth_user.user = blanc_user;
}
