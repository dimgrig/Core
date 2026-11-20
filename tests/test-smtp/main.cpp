#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <core-io-lib/SmtpClient.h>

//cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCORE_SDK_ROOT=/home/dima/.local/core/qt6/latest/
//cmake --build build -j$(nproc)
//./build/test-smtp


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // ── Конфигурация SMTP ───────────────────────────────────────────
    // ЗАМЕНИ НА РЕАЛЬНЫЕ ДАННЫЕ!
    SmtpConfig config;
    config.host =       "smtp.mail.ru";           // Или smtp.mail.ru, smtp.yandex.ru
    config.port =       465;                      // 587 для STARTTLS, 465 для SSL
    //config.useSsl = false;                      // false для port 587 (STARTTLS)
    //config.useStartTls = true;                  // true для port 587
    config.useSsl =     true;
    config.useStartTls = false;
    config.from =       "test@mail.ru";    // Твой email
    config.username =   "test@mail.ru";    // Логин (обычно=email)
    config.password =   "test";   // Пароль приложения (не пароль от почты!)
    config.timeoutMs = 30000;
    
    // ── Создаём тестовое вложение ───────────────────────────────────
    QString attachmentPath = "/tmp/test-attachment.txt";
    {
        QFile file(attachmentPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "Это тестовое вложение.\n";
            out << "Отправлено из test-smtp программы.\n";
            out << "Время: " << QDateTime::currentDateTime().toString() << "\n";
            file.close();
            qDebug() << "Создан файл:" << attachmentPath;
        } else {
            qCritical() << "Не удалось создать файл:" << file.errorString();
            return 1;
        }
    }
    
    // ── Создаём SmtpClient ──────────────────────────────────────────
    SmtpClient client;
    client.setConfig(config);
    
    // ── Подключаем сигналы ──────────────────────────────────────────
    QObject::connect(&client, &SmtpClient::mailSent, [](const QString &to) {
        qDebug() << "✓ Письмо успешно отправлено:" << to;
        QCoreApplication::quit();
    });
    
    QObject::connect(&client, &SmtpClient::mailFailed, [](const QString &to, const QString &error) {
        qCritical() << "✗ Ошибка отправки письма:" << to;
        qCritical() << "  Причина:" << error;
        QCoreApplication::exit(1);
    });
    
    QObject::connect(&client, &SmtpClient::stateChanged, [](const QString &state) {
        qDebug() << "Состояние:" << state;
    });
    
    // ── Формируем письмо ────────────────────────────────────────────
    SmtpMessage message;
    message.to = "test@mail.ru";     // Кому отправить
    message.subject = "Тест отправки из Qt SmtpClient";
    message.body = "Привет!\n\nЭто тестовое письмо с вложением.\n\nОтправлено из test-smtp.";
    message.attachments = {attachmentPath};
    
    // ── Отправляем ──────────────────────────────────────────────────
    qDebug() << "Отправка письма...";
    client.sendMail(message);
    
    // Таймаут на весь процесс (60 секунд)
    QTimer::singleShot(60000, &app, [&app]() {
        qCritical() << "Таймаут: письмо не отправлено за 60 секунд";
        app.exit(2);
    });
    
    return app.exec();
}