#include "TestSmtpClient.h"
#include <QTemporaryFile>

void TestSmtpClient::initTestCase() {
    _server = new QTcpServer(this);
    //_server->setSocketOption(QAbstractSocket::LowDelayOption, 1); // TCP_NODELAY
}

void TestSmtpClient::cleanupTestCase() {
    stopMockSmtpServer();
}

void TestSmtpClient::init() {
    _receivedCommands.clear();
    _requireAuth = false;
    stopMockSmtpServer();
    QTest::qWait(50); // Даём сокетам закрыться
}

void TestSmtpClient::cleanup() {
    stopMockSmtpServer();
    QTest::qWait(100); // Даём портам освободиться
}

quint16 TestSmtpClient::startMockSmtpServer(bool requireAuth) {
    _requireAuth = requireAuth;
    stopMockSmtpServer();
    _server->disconnect(this);

    if (!_server->listen(QHostAddress::LocalHost, 0)) {
        qWarning() << "Failed to listen:" << _server->errorString();
        return 0;
    }

    quint16 actualPort = _server->serverPort();

    connect(_server, &QTcpServer::newConnection, this, [this]() {
        QTcpSocket *client = _server->nextPendingConnection();
        connect(client, &QTcpSocket::disconnected, client, &QObject::deleteLater);

        connect(client, &QTcpSocket::readyRead, client,
                [this, client, buffer = QByteArray(), authPhase = false, authStep = 0, inDataPhase = false]() mutable {
                    buffer += client->readAll();

                    if (inDataPhase) {
                        if (buffer == ".\r\n" || buffer.endsWith("\r\n.\r\n")) {
                            _receivedCommands.append(buffer.trimmed());
                            buffer.clear();
                            inDataPhase = false;
                            client->write("250 OK\r\n");
                            client->flush();
                        }
                        return;
                    }

                    while (buffer.contains("\r\n")) {
                        int idx = buffer.indexOf("\r\n");
                        QByteArray line = buffer.left(idx);
                        buffer.remove(0, idx + 2);

                        _receivedCommands.append(line.trimmed());
                        QString cmd = QString::fromUtf8(line).trimmed();
                        QString cmdUpper = cmd.toUpper();

                        if (cmdUpper.startsWith("EHLO")) {
                            if (_requireAuth) {
                                client->write("250-localhost\r\n250-AUTH LOGIN PLAIN\r\n250 OK\r\n");
                            } else {
                                client->write("250 localhost\r\n");
                            }
                        } else if (cmdUpper.startsWith("AUTH LOGIN")) {
                            authPhase = true;
                            authStep = 0;
                            client->write("334 VXNlcm5hbWU6\r\n");
                        } else if (authPhase) {
                            if (authStep == 0) {
                                authStep = 1;
                                client->write("334 UGFzc3dvcmQ6\r\n");
                            } else {
                                authPhase = false;
                                client->write("235 Authentication successful\r\n");
                            }
                        } else if (cmdUpper.startsWith("MAIL FROM")) {
                            client->write("250 OK\r\n");
                        } else if (cmdUpper.startsWith("RCPT TO")) {
                            client->write("250 OK\r\n");
                        } else if (cmdUpper.startsWith("DATA")) {
                            inDataPhase = true;
                            client->write("354 Start mail input\r\n");
                            client->flush();
                            if (buffer == ".\r\n" || buffer.endsWith("\r\n.\r\n")) {
                                _receivedCommands.append(buffer.trimmed());
                                buffer.clear();
                                inDataPhase = false;
                                client->write("250 OK\r\n");
                            }
                        } else if (cmdUpper.startsWith("QUIT")) {
                            client->write("221 Bye\r\n");
                            client->flush();
                            client->disconnectFromHost();
                            return;
                        }
                        client->flush();
                    }
                });

        client->write("220 localhost SMTP Mock Server\r\n");
        client->flush();
    });

    return actualPort;
}

void TestSmtpClient::stopMockSmtpServer() {
    if (_server->isListening()) {
        _server->close();
    }
}

void TestSmtpClient::testSendMailWithoutAuth() {
    quint16 port = startMockSmtpServer(false);

    SmtpClient client;
    SmtpConfig config;
    config.host = "localhost";
    config.port = port;
    config.from = "test@example.com";
    config.useSsl = false;
    config.useStartTls = false;
    config.timeoutMs = 5000;

    client.setConfig(config);

    QSignalSpy sentSpy(&client, &SmtpClient::mailSent);
    QSignalSpy failedSpy(&client, &SmtpClient::mailFailed);

    client.sendMail("recipient@example.com", "Test Subject", "Test Body");

    QVERIFY2(sentSpy.wait(10000), "Mail was not sent");
    QCOMPARE(sentSpy.count(), 1);
    QCOMPARE(failedSpy.count(), 0);

    QVERIFY(_receivedCommands.size() >= 5);
}

void TestSmtpClient::testSendMailWithAuth() {
    quint16 port = startMockSmtpServer(true);

    SmtpClient client;
    SmtpConfig config;
    config.host = "localhost";
    config.port = port;
    config.from = "test@example.com";
    config.username = "testuser";
    config.password = "testpass";
    config.useSsl = false;
    config.useStartTls = false;
    config.timeoutMs = 5000;

    client.setConfig(config);

    QSignalSpy sentSpy(&client, &SmtpClient::mailSent);

    client.sendMail("recipient@example.com", "Test", "Body");

    QVERIFY2(sentSpy.wait(10000), "Mail with auth was not sent");
    QCOMPARE(sentSpy.count(), 1);
}

void TestSmtpClient::testSendMailWithAttachment() {
    quint16 port = startMockSmtpServer(false);

    QTemporaryFile tempFile;
    tempFile.setAutoRemove(true);
    QVERIFY(tempFile.open());
    tempFile.write("Test attachment content");
    tempFile.close();

    SmtpClient client;
    SmtpConfig config;
    config.host = "localhost";
    config.port = port;
    config.from = "test@example.com";
    config.useSsl = false;
    config.useStartTls = false;
    config.timeoutMs = 5000;

    client.setConfig(config);

    QSignalSpy sentSpy(&client, &SmtpClient::mailSent);

    QStringList attachments = {tempFile.fileName()};
    client.sendMail("recipient@example.com", "Test", "Body", attachments);

    QVERIFY2(sentSpy.wait(10000), "Mail with attachment was not sent");
    QCOMPARE(sentSpy.count(), 1);
}

void TestSmtpClient::testConnectionTimeout() {
    SmtpClient client;
    SmtpConfig config;
    config.host = "192.0.2.1"; // TEST-NET - гарантированно не отвечает
    config.port = 25;
    config.timeoutMs = 1000;

    client.setConfig(config);

    QSignalSpy failedSpy(&client, &SmtpClient::mailFailed);

    client.sendMail("test@example.com", "Test", "Body");

    QVERIFY2(failedSpy.wait(5000), "Timeout was not triggered");
    QCOMPARE(failedSpy.count(), 1);
}

void TestSmtpClient::testQueueProcessing() {
    quint16 port = startMockSmtpServer(false);

    SmtpClient client;
    SmtpConfig config;
    config.host = "localhost";
    config.port = port;
    config.from = "test@example.com";
    config.useSsl = false;
    config.useStartTls = false;
    config.timeoutMs = 10000;

    client.setConfig(config);

    QSignalSpy sentSpy(&client, &SmtpClient::mailSent);
    QSignalSpy allProcessedSpy(&client, &SmtpClient::allMailsProcessed);

    client.sendMail("recipient1@example.com", "Test 1", "Body 1");
    client.sendMail("recipient2@example.com", "Test 2", "Body 2");
    client.sendMail("recipient3@example.com", "Test 3", "Body 3");

    QVERIFY2(allProcessedSpy.wait(30000), "Not all mails were processed");
    QCOMPARE(sentSpy.count(), 3);
    QCOMPARE(client.queueSize(), 0);
}
