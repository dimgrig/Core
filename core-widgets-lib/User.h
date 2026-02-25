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
#include "IUserRoleManager.h"
#include "Logger.h"

#include "core-widgets-lib-global.h"

const int UserCharLength = 256;
const std::string UserDefaultFilename = "user.dat";

#define auth_user User_singleton::instance()

void UserTest();

struct CORE_WIDGETS_LIB_EXPORT User_s {
    char name[UserCharLength] = {};
    char level[UserCharLength] = {};
    char password[UserCharLength] = {};

    std::string get_text() const {return std::string(level)  + ": " + std::string(name);} //
    std::string get_level() const {return std::string(level);} // + ": " + QString(name)
    void from_text(QString text) {
        QStringList sl = text.split(": ", SKIP_EMPTY_PARTS); //Qt::
        if (sl.size() == 2) {
            strncpy(this->name, sl.at(1).toStdString().c_str(), UserCharLength);
            strncpy(this->level, sl.at(0).toStdString().c_str(), UserCharLength);
        }
    }
};

class CORE_WIDGETS_LIB_EXPORT User_singleton {
public:
    static User_singleton& instance() {
        static User_singleton inst;
        return inst;
    }

    void setRoleManager(IUserRoleManager* roleManager) {_roleManager = roleManager;};
    bool isAdmin(QString userRole = "") const {
        if (userRole == "") userRole = QString::fromStdString(user.get_level());
        return _roleManager ? _roleManager->isAdmin(userRole) : false;
    }
    bool isDeveloper(QString userRole = "") const {
        if (userRole == "") userRole = QString::fromStdString(user.get_level());
        return _roleManager ? _roleManager->isDeveloper(userRole) : false;
    }
    bool checkUserRole(const QString &requiredUserRole, QString userRole = "") const {
        if (userRole == "") userRole = QString::fromStdString(user.get_level());
        return _roleManager ? _roleManager->checkUserRole(requiredUserRole, userRole) : false;
    }
    int roleToInt() const {
        return _roleManager ? _roleManager->roleToInt(QString::fromStdString(user.get_level())) : 0;
    }
    bool isFeatureEnabled(const QString& featureName) const {
        return _roleManager ? _roleManager->isFeatureEnabled(featureName, QString::fromStdString(user.get_level())) : false;
    }

    User_s user;
private:
    User_singleton() {}
    User_singleton(const User_singleton&) = delete;
    User_singleton& operator=(const User_singleton&) = delete;
    ~User_singleton() {}

    IUserRoleManager* _roleManager;
};

namespace Ui {
    class User;
    class Users;
}

class CORE_WIDGETS_LIB_EXPORT User : public QDialog {
Q_OBJECT
public:
    User(IUserRoleManager* roleManager, bool editing = true, QWidget *parent = nullptr);
    ~User();

    void set_user(const std::string& name, const std::string& level);
signals:
    void save_user(const char *name, const char *level, const char *password);
private slots:
    void on_save();
private:
    Ui::User *ui;

    IUserRoleManager* _roleManager;
};

class CORE_WIDGETS_LIB_EXPORT Users : public QDialog {
Q_OBJECT
public:
    Users(IUserRoleManager* roleManager = nullptr,
          const std::string& filename = UserDefaultFilename,
          QWidget *parent = nullptr);
    ~Users();

    void load();
    void save();

    void addUser(const char *name, const char *level, const char *password);
    void deleteUser(const char *name, const char *level, const char *password);

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
    std::string _filename;

    std::vector<User*> user_v;

    IUserRoleManager* _roleManager;

    bool _adding = false;
    bool _editing = false;
};

#endif // USER_H
