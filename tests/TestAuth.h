#ifndef TESTAUTH_H
#define TESTAUTH_H

#include <QtTest/QtTest>
#include <QSignalSpy>

#include "Auth.h"

class TestAuthWithPublicUsers : public Auth {
public:
    using Auth::_users; // делает _users доступным
    TestAuthWithPublicUsers(IUserRoleManager* mgr) : Auth(mgr) {}
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
};

#endif // TESTAUTH_H
