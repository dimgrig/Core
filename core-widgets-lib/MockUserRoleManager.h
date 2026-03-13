#ifndef MOCKUSERROLEMANAGER_H
#define MOCKUSERROLEMANAGER_H

#include <QHash>
#include <QMap>

#include "IUserRoleManager.h"

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT MockUserRoleManager : public IUserRoleManager {
public:
    enum class Role {
        Undefined = 0,
        Developer,
        Admin,
        Operator
    };

    MockUserRoleManager();

    // IUserRoleManager interface
    bool canAddUser(const QString& currentUserRole) const override;
    bool canDeleteUser(const QString& currentUserRole, const QString& targetUserRole) const override;
    bool canEditUser(const QString& currentUserRole, const QString& targetUserRole) const override;
    bool canViewUsers(const QString& currentUserRole) const override;

    bool isAdmin(const QString& userRole) const override;
    bool isDeveloper(const QString& userRole) const override;
    bool checkUserRole(const QString& requiredUserRole, const QString& userRole) const override;

    int roleToInt(const QString& userRole) const override;

    QStringList getAllRoles() const override;
    QStringList getRolesForAdding() const override;
    QStringList getRolesForEditing() const override;
    QString getDefaultRole() const override;

    bool isFeatureEnabled(const QString& featureName, const QString& userRole) const override;
private:
    Role roleFromString(const QString& roleStr) const;
    QString roleToString(Role role) const;

    QHash<QString, Role> _stringToRole;
    QHash<Role, QString> _roleToString;
    QList<Role> _allRoles;
};

inline uint qHash(MockUserRoleManager::Role key, uint seed = 0) noexcept
{
    return qHash(static_cast<int>(key), seed);
}

#endif // MOCKUSERROLEMANAGER_H
