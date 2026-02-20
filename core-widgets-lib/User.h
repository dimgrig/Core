#ifndef USER_H
#define USER_H

#include <vector>
#include <fstream>
#include <cstring>

#include <QDialog>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>

#include "aqp.h"
#include "CoreConfig.h"
#include "enum_macro.h"
#include "Logger.h"

#include "core-widgets-lib-global.h"

const int Length = 256;

ENUM_MACRO4(AuthUserRole, AuthUserRole_Undefined, Developer, Admin, Operator,
            Undefined, Developer, Admin, Operator,
            Не определено, Разработчик, Администратор, Пользователь,
            0, 1, 2, 3);

static QStringList AuthUserRolesAll = {AuthUserRole_to_text(Operator),
                                       AuthUserRole_to_text(Admin),
                                       AuthUserRole_to_text(Developer)};
static QStringList AuthUserRolesEdit = {AuthUserRole_to_text(Operator),
                                        AuthUserRole_to_text(Admin)};
static QStringList AuthUserRolesAdd = {AuthUserRole_to_text(Operator)};

#define auth_user User_singleton::instance()

struct CORE_WIDGETS_LIB_EXPORT User_s {
    char name[Length] = {};
    char level[Length] = {};
    char password[Length] = {};

    std::string get_text() const {return std::string(level)  + ": " + std::string(name);} //
    std::string get_level() const {return std::string(level);} // + ": " + QString(name)
    void from_text(QString text) {
        QStringList sl = text.split(": ", SKIP_EMPTY_PARTS); //Qt::
        if (sl.size() == 2) {
            strncpy(this->name, sl.at(1).toStdString().c_str(), Length);
            strncpy(this->level, sl.at(0).toStdString().c_str(), Length);
        }
    }
};

class CORE_WIDGETS_LIB_EXPORT User_singleton {
public:
    static User_singleton& instance() {
        static User_singleton inst;
        return inst;
    }

    User_s user;
private:
    User_singleton() {}
    User_singleton(const User_singleton&) = delete;
    User_singleton& operator=(const User_singleton&) = delete;
    ~User_singleton() {}
};

namespace Ui {
    class User;
    class Users;
}

class CORE_WIDGETS_LIB_EXPORT User : public QDialog {
Q_OBJECT
public:
    User(bool editing = true, QWidget *parent = nullptr);
    ~User();

    void set_user(const std::string& name, const std::string& level);
signals:
    void save_user(const char *name, const char *level, const char *password);
private slots:
    void on_save();
private:
    Ui::User *ui;
};

class CORE_WIDGETS_LIB_EXPORT Users : public QDialog {
Q_OBJECT
public:
    Users(QWidget *parent = nullptr);
    ~Users();

    void load();
    void save();

    void add(const char *name, const char *level, const char *password);

    User_s* find(const std::string& name);
    User_s* find(const std::string& name, const std::string& password);

    //std::vector<User_s> get() {return _users;};
    QStringList get_list() const;

signals:
    void user_fail();
    void password_fail();
public slots:
    void show();
private slots:
    void _save();
    void _add();
    void _delete();
    void double_clicked(const QModelIndex& index);
    void save_user(const char *name, const char *level, const char *password);
private:
    Ui::Users *ui;

    std::string hash(QString s);

    void update_lw();

    std::vector<User_s> _users;
    User_s read_user(std::ifstream &fst);
    const std::string filename = "user.dat";

    std::vector<User*> user_v;

    bool _adding = false;
    bool _editing = false;
};

#endif // USER_H
