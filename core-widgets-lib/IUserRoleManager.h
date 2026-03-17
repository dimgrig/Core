#ifndef IUSERROLEMANAGER_H
#define IUSERROLEMANAGER_H

#include <QStringList>

class IUserRoleManager {
public:
    virtual ~IUserRoleManager() = default;

    // Проверка прав текущего пользователя (роль передаётся как QString)
    virtual bool canAddUser(const QString& currentUserRole) const = 0;
    virtual bool canDeleteUser(const QString& currentUserRole, const QString& targetUserRole) const = 0;
    virtual bool canEditUser(const QString& currentUserRole, const QString& targetUserRole) const = 0;
    virtual bool canViewUsers(const QString& currentUserRole) const = 0;

    virtual bool isAdmin(const QString& userRole) const = 0;
    virtual bool isDeveloper(const QString& userRole) const = 0;
    virtual bool checkUserRole(const QString& requiredUserRole, const QString& userRole) const = 0;

    virtual int roleToInt(const QString& userRole) const = 0;

    // Списки ролей для пользовательского интерфейса
    virtual QStringList getAllRoles() const = 0;                            // все существующие роли
    virtual QStringList getAllRolesExcludeDeveloper() const = 0;            // все существующие роли кроме разработчика
    virtual QStringList getRolesForAdding() const = 0;                      // роли, доступные для назначения новому пользователю
    virtual QStringList getRolesForEditing() const = 0;                     // роли, которые можно редактировать (обычно совпадают с adding)
    virtual QString getDefaultRole() const = 0;                             // роль по умолчанию для новых пользователей

    virtual bool isFeatureEnabled(const QString& featureName, const QString& userRole) const = 0;
};

#endif // IUSERROLEMANAGER_H
