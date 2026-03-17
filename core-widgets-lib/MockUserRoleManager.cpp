#include "MockUserRoleManager.h"

MockUserRoleManager::MockUserRoleManager() {
    _allRoles = {Role::Operator, Role::Admin, Role::Developer};
    _roleToString = {
        {Role::Developer,   "Разработчик"},
        {Role::Admin,       "Администратор"},
        {Role::Operator,    "Пользователь"}
    };
    for (auto it = _roleToString.begin(); it != _roleToString.end(); ++it)
        _stringToRole[it.value()] = it.key();
}

MockUserRoleManager::Role MockUserRoleManager::roleFromString(const QString& roleStr) const {
    return _stringToRole.value(roleStr, Role::Undefined);
}

QString MockUserRoleManager::roleToString(Role role) const {
    return _roleToString.value(role, QString());
}

bool MockUserRoleManager::canAddUser(const QString& currentUserRole) const {
    Role cur = roleFromString(currentUserRole);
    return cur == Role::Admin || cur == Role::Developer;
}

bool MockUserRoleManager::canDeleteUser(const QString& currentUserRole, const QString& targetUserRole) const {
    Role cur = roleFromString(currentUserRole);
    Role tgt = roleFromString(targetUserRole);
    if (cur == Role::Developer) return true; // разработчик может удалять всех
    if (cur == Role::Admin) return tgt != Role::Developer && tgt != Role::Admin; // админ не удаляет админов и разработчиков
    return false;
}

bool MockUserRoleManager::canEditUser(const QString& currentUserRole, const QString& targetUserRole) const {
    Role cur = roleFromString(currentUserRole);
    Role tgt = roleFromString(targetUserRole);
    if (cur == Role::Developer) return true;
    if (cur == Role::Admin) return tgt != Role::Developer; // админ редактирует всех, кроме разработчика
    return false;
}

bool MockUserRoleManager::canViewUsers(const QString& currentUserRole) const {
    return roleFromString(currentUserRole) != Role::Undefined;
}

bool MockUserRoleManager::isAdmin(const QString& userRole) const {
    Role role = roleFromString(userRole);
    return role == Role::Admin;
}

bool MockUserRoleManager::isDeveloper(const QString& userRole) const {
    Role role = roleFromString(userRole);
    return role == Role::Developer;
}

bool MockUserRoleManager::checkUserRole(const QString& requiredUserRole, const QString& userRole) const {
    return requiredUserRole == userRole;
}

int MockUserRoleManager::roleToInt(const QString& roleStr) const {
    Role role = roleFromString(roleStr);
    return static_cast<int>(role);
}

QStringList MockUserRoleManager::getAllRoles() const {
    QStringList list;
    for (Role r : _allRoles)
        list << roleToString(r);
    return list;
}

QStringList MockUserRoleManager::getRolesForAdding() const {
    QStringList list;
    list << roleToString(Role::Operator) << roleToString(Role::Admin);
    return list;
}

QStringList MockUserRoleManager::getRolesForEditing() const {
    return getRolesForAdding();
}

QString MockUserRoleManager::getDefaultRole() const {
    return roleToString(Role::Operator);
}

bool MockUserRoleManager::isFeatureEnabled(const QString& featureName, const QString& userRole) const {
    Q_UNUSED(featureName);
    Q_UNUSED(userRole);
    return false;
}
