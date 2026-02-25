#include "TestAuth.h"

#include "MockUserRoleManager.h"

void TestAuth::initTestCase()
{

}

void TestAuth::testAuthSuccess() {
    MockUserRoleManager mock;
    Auth auth(&mock);

    // Предварительно добавляем пользователя через внутренний _users
    // Для доступа к _users нужно либо сделать его публичным, либо использовать friend.
    // В тестах можно создать подкласс:

    TestAuthWithPublicUsers testAuth(&mock);
    testAuth._users->addUser("testuser", "Администратор", "correct");

    // Эмулируем ввод логина и пароля
    QSignalSpy okSpy(&testAuth, &Auth::auth_ok);
    QSignalSpy failedSpy(&testAuth, &Auth::auth_failed);
    testAuth.enter("testuser", "correct");
    testAuth.accept(); // вызов accept проверит пароль и испустит сигнал

    QCOMPARE(okSpy.count(), 1);
    QCOMPARE(failedSpy.count(), 0);
    QCOMPARE(std::string(auth_user.user.name), "testuser");
}

void TestAuth::testAuthWrongPassword() {
    MockUserRoleManager mock;
    TestAuthWithPublicUsers testAuth(&mock);
    testAuth._users->addUser("testuser", "Администратор", "correct");

    testAuth.exit();

    QSignalSpy okSpy(&testAuth, &Auth::auth_ok);
    QSignalSpy failedSpy(&testAuth, &Auth::auth_failed);
    testAuth.enter("testuser", "wrong");
    testAuth.accept();

    QCOMPARE(okSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(std::string(auth_user.user.name), ""); // пользователь не установлен
}

void TestAuth::testAuthUnknownUser() {
    MockUserRoleManager mock;
    TestAuthWithPublicUsers testAuth(&mock);
    testAuth._users->addUser("testuser", "Администратор", "correct");

    QSignalSpy okSpy(&testAuth, &Auth::auth_ok);
    QSignalSpy failedSpy(&testAuth, &Auth::auth_failed);
    testAuth.enter("unknown", "any");
    testAuth.accept();

    QCOMPARE(okSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);
}
