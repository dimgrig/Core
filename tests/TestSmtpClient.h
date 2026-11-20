#ifndef TESTSMTPCLIENT_H
#define TESTSMTPCLIENT_H

#include <QObject>
#include <QtTest/QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSignalSpy>
#include <core-io-lib/SmtpClient.h>

class TestSmtpClient : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testSendMailWithoutAuth();
    void testSendMailWithAuth();
    void testSendMailWithAttachment();
    void testConnectionTimeout();
    void testQueueProcessing();

private:
    // Запускает сервер на динамическом порте, возвращает реальный порт
    quint16 startMockSmtpServer(bool requireAuth = false);
    void stopMockSmtpServer();

    QTcpServer *_server;
    QList<QByteArray> _receivedCommands;
    bool _requireAuth;
};

#endif // TESTSMTPCLIENT_H
