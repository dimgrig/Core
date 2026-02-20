#ifndef AUTH_H
#define AUTH_H

#include <fstream>

#include <QDialog>
#include <QPropertyAnimation>

#include "User.h"

namespace Ui {
    class Auth;
}

class Auth : public QDialog {
Q_OBJECT
public:
    Auth(QWidget *parent = nullptr);
    //explicit Auth(Users* users, QWidget *parent = nullptr);
    ~Auth();
signals:
    void auth_ok();
public slots:
    //void set_users(Users *users);
    void users_show();
    void enter(std::string login, std::string password);

    void user_fail();
    void password_fail();

    void accept() override;
    void reject() override;

    void exit();
private slots:
    //void enter();
    void on_shake();
private:
    Ui::Auth *ui;

    Users* _users = new Users(this);
};



#endif // AUTH_H
