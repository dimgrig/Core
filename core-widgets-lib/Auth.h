#ifndef AUTH_H
#define AUTH_H

#include <fstream>

#include <QDialog>
#include <QPropertyAnimation>

#include "User.h"

#include "core-widgets-lib-global.h"

namespace Ui {
    class Auth;
}

class CORE_WIDGETS_LIB_EXPORT Auth : public QDialog {
Q_OBJECT
public:
    Auth(IUserRoleManager* roleManager, QWidget *parent = nullptr);
    ~Auth();
signals:
    void auth_ok();
    void auth_failed();
public slots:
    void users_show();
    void enter(std::string login, std::string password);

    void user_fail();
    void password_fail();

    void accept() override;
    void reject() override;

    void exit();

    friend class TestAuthWithPublicUsers;
private slots:
    void on_shake();
    void onAuthOk();
    void onAuthFailed();
private:
    Ui::Auth *ui;

    IUserRoleManager *_roleManager;
    Users* _users;
};



#endif // AUTH_H
