#ifndef TESTAUTH_H
#define TESTAUTH_H

#include <QtTest/QtTest>
#include <QSignalSpy>

#include <core-widgets-lib/Auth.h>

class TestAuthWithPublicUsers : public Auth {
public:
    using Auth::_users; // делает _users доступным
    TestAuthWithPublicUsers(IUserRoleManager* mgr) : Auth(mgr) {}

    Users* getUsers() const { return _users; }
};

class TestAuth : public QObject
{
    Q_OBJECT
public:

private slots:
    void initTestCase();

    void testAuthSuccess();
    void testAuthWrongPassword();
    void testAuthUnknownUser();

    void testSetAndGetDefaultUser();
    void testTryAutoLoginSuccess();
    void testTryAutoLoginWrongPassword();
    void testTryAutoLoginDefault();
    void testClearDefaultUser();
    void testGetUsersExceptDeveloper();
private:
    QString _appName = "TestDefaultUser";
};


#endif // TESTAUTH_H
