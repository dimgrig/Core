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


void TestAuth::testSetAndGetDefaultUser()
{
    // Очищаем настройки перед тестом
    QSettings settings(__ORGANIZATION_NAME, _appName);
    settings.clear();

    // Удаляем старый файл пользователей, если существует
    QFile::remove(UserDefaultFilename.c_str());

    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open();
    Users users(&mock, tempFile.fileName().toStdString());

    // Добавляем пользователей (не обязательно, но для реализма)
    users.addUser("user1", "Администратор", "pass1");
    users.addUser("user2", "Пользователь", "pass2");

    // Начально сохранён пользователь admin
    QString login, password;
    QVERIFY(users.getDefaultUser(login, password));
    QCOMPARE(login, QString("admin"));
    QCOMPARE(password, QString("123"));

    // Устанавливаем пользователя по умолчанию
    users.setDefaultUser("user1", "pass1");

    QVERIFY(users.getDefaultUser(login, password));
    QCOMPARE(login, QString("user1"));
    QCOMPARE(password, QString("pass1"));

    // Очищаем
    users.clearDefaultUser();
    QVERIFY(!users.getDefaultUser(login, password));

    // Восстанавливаем настройки и удаляем тестовые
    settings.clear();
}

void TestAuth::testTryAutoLoginSuccess()
{
    QSettings settings(__ORGANIZATION_NAME, _appName);
    settings.clear();

    // Удаляем старый файл пользователей, если существует
    QFile::remove(UserDefaultFilename.c_str());

    // Создаём временный файл с тестовым пользователем
    MockUserRoleManager mock;
    Users tempUsers(&mock, UserDefaultFilename);
    tempUsers.addUser("testuser", "Администратор", "correctpass");
    tempUsers.save();  // сохраняем в файл по умолчанию

    // Создаём Auth – он загрузит пользователей из файла по умолчанию
    TestAuthWithPublicUsers auth(&mock);
    Users* authUsers = auth.getUsers();

    // Сохраняем этого пользователя как пользователя по умолчанию
    authUsers->setDefaultUser("testuser", "correctpass");

    // Пробуем автоматический вход
    bool result = auth.tryAutoLogin();
    QVERIFY(result);

    // Проверяем, что текущий пользователь установлен
    QCOMPARE(QString::fromStdString(auth_user.user.name), QString("testuser"));
    QCOMPARE(QString::fromStdString(auth_user.user.level), QString("Администратор"));

    // Очистка
    auth_user.user = User_s();
    authUsers->clearDefaultUser();
    QFile::remove(UserDefaultFilename.c_str());
}

void TestAuth::testTryAutoLoginWrongPassword()
{
    QSettings settings(__ORGANIZATION_NAME, _appName);
    settings.clear();

    QFile::remove(UserDefaultFilename.c_str());

    MockUserRoleManager mock;
    Users tempUsers(&mock, UserDefaultFilename);
    tempUsers.addUser("testuser", "Администратор", "correctpass");
    tempUsers.save();

    TestAuthWithPublicUsers auth(&mock);
    Users* authUsers = auth.getUsers();

    // Сохраняем неверный пароль
    authUsers->setDefaultUser("testuser", "wrongpass");

    bool result = auth.tryAutoLogin();
    QVERIFY(!result);

    // Текущий пользователь не должен измениться (пустой)
    QCOMPARE(std::string(auth_user.user.name), std::string());

    authUsers->clearDefaultUser();
    QFile::remove(UserDefaultFilename.c_str());
}

void TestAuth::testTryAutoLoginDefault()
{
    QSettings settings(__ORGANIZATION_NAME, _appName);
    settings.clear();

    QFile::remove(UserDefaultFilename.c_str());

    MockUserRoleManager mock;
    Users tempUsers(&mock, UserDefaultFilename);
    tempUsers.addUser("testuser", "Администратор", "pass");
    tempUsers.save();

    TestAuthWithPublicUsers auth(&mock);
    // Не устанавливаем пользователя по умолчанию

    bool result = auth.tryAutoLogin();
    QVERIFY(result);

    QFile::remove(UserDefaultFilename.c_str());
}

void TestAuth::testClearDefaultUser()
{
    QSettings settings(__ORGANIZATION_NAME, _appName);

    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open();
    Users users(&mock, tempFile.fileName().toStdString());

    users.setDefaultUser("someuser", "somepass");

    QString login, pass;
    QVERIFY(users.getDefaultUser(login, pass));
    QCOMPARE(login, "someuser");
    QCOMPARE(pass, "somepass");

    users.clearDefaultUser();
    QVERIFY(!users.getDefaultUser(login, pass));
}

void TestAuth::testGetUsersExceptDeveloper()
{
    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open();
    Users users(&mock, tempFile.fileName().toStdString());

    users.addUser("dev", "Разработчик", "p");
    users.addUser("admin", "Администратор", "p");
    users.addUser("user", "Пользователь", "p");

    QStringList list = users.getUsersExceptDeveloper();
    QCOMPARE(list.size(), 2);
    QVERIFY(list.contains("Администратор: admin"));
    QVERIFY(list.contains("Пользователь: user"));
    QVERIFY(!list.contains("Разработчик: dev"));
}
