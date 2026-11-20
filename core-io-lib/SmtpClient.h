#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSslSocket>
#include <QQueue>
#include <QTimer>
#include "core-io-lib-global.h"

struct CORE_IO_LIB_EXPORT SmtpConfig {
    QString host;
    quint16 port = 587;
    QString username;
    QString password;
    QString from;
    bool useSsl = false;        // SSL/TLS с самого начала (порт 465)
    bool useStartTls = true;    // STARTTLS после EHLO (порт 587)
    int timeoutMs = 30000;      // Таймаут на каждый этап
};

struct CORE_IO_LIB_EXPORT SmtpMessage {
    QString to;
    QString subject;
    QString body;
    QStringList attachments;    // Абсолютные пути к файлам
};

class CORE_IO_LIB_EXPORT SmtpClient : public QObject {
    Q_OBJECT
public:
    explicit SmtpClient(QObject *parent = nullptr);
    ~SmtpClient();

    void setConfig(const SmtpConfig &config);
    SmtpConfig config() const;

    // Поставить письмо в очередь на отправку
    void sendMail(const SmtpMessage &message);
    
    // Удобный overload
    void sendMail(const QString &to, const QString &subject, 
                  const QString &body, const QStringList &attachments = {});

    // Очистить очередь (текущее письмо будет прервано)
    void clearQueue();

    bool isSending() const;
    int queueSize() const;

signals:
    void mailSent(const QString &to);
    void mailFailed(const QString &to, const QString &errorMsg);
    void stateChanged(const QString &state);
    void allMailsProcessed();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onSocketEncrypted();
    void onTimeout();

private:
    enum class State {
        Idle,
        WaitingForGreeting,
        SendingEhlo,
        WaitingForEhloResponse,
        SendingStartTls,
        WaitingForStartTlsResponse,
        SendingAuth,
        WaitingForAuthResponse,
        SendingMailFrom,
        WaitingForMailFromResponse,
        SendingRcptTo,
        WaitingForRcptToResponse,
        SendingData,
        WaitingForDataResponse,
        SendingMessage,
        WaitingForMessageResponse,
        SendingQuit,
        WaitingForQuitResponse
    };

    void processQueue();
    void sendNextCommand();
    void sendCommand(const QByteArray &command);
    void sendRawData(const QByteArray &data);
    void changeState(State newState);
    void resetSession();
    void failCurrentMail(const QString &errorMsg);
    void succeedCurrentMail();
    
    QByteArray buildMessage() const;
    QByteArray encodeBase64(const QByteArray &data) const;
    QString extractFileName(const QString &filePath) const;
    
    bool verifyResponseCode(const QByteArray &response, int expectedCode) const;
    int extractResponseCode(const QByteArray &response) const;

    SmtpConfig _config;
    QSslSocket *_socket;
    QTimer *_timeoutTimer;
    QQueue<SmtpMessage> _queue;
    SmtpMessage _currentMessage;
    State _state;
    QByteArray _responseBuffer;
    bool _tlsStarted;
    bool _authenticated;
};

#endif // SMTPCLIENT_H
