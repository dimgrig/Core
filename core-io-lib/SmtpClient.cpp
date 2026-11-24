#include "SmtpClient.h"
#include "Logger.h"
#include "Categories.h"
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QRandomGenerator>
#include <QUrl>

SmtpClient::SmtpClient(QObject *parent)
    : QObject(parent)
    , _socket(new QSslSocket(this))
    , _timeoutTimer(new QTimer(this))
    , _state(State::Idle)
    , _tlsStarted(false)
    , _authenticated(false)
{
    _timeoutTimer->setSingleShot(true);

    connect(_socket, &QSslSocket::connected, this, &SmtpClient::onSocketConnected);
    connect(_socket, &QSslSocket::disconnected, this, &SmtpClient::onSocketDisconnected);
    connect(_socket, &QSslSocket::errorOccurred, this, &SmtpClient::onSocketError);
    connect(_socket, &QSslSocket::readyRead, this, &SmtpClient::onSocketReadyRead);
    connect(_socket, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
            this, &SmtpClient::onSslErrors);
    connect(_socket, &QSslSocket::encrypted, this, &SmtpClient::onSocketEncrypted);
    connect(_timeoutTimer, &QTimer::timeout, this, &SmtpClient::onTimeout);
}

SmtpClient::~SmtpClient() {
    if (_socket->state() != QAbstractSocket::UnconnectedState) {
        _socket->disconnectFromHost();
    }
}

void SmtpClient::setConfig(const SmtpConfig &config) {
    _config = config;
}

SmtpConfig SmtpClient::config() const {
    return _config;
}

void SmtpClient::sendMail(const SmtpMessage &message) {
    _queue.enqueue(message);
    qCDebug(logDebug) << "SmtpClient: письмо добавлено в очередь, размер:" << _queue.size();

    if (_state == State::Idle) {
        processQueue();
    }
}

void SmtpClient::sendMail(const QString &to, const QString &subject,
                          const QString &body, const QStringList &attachments) {
    SmtpMessage msg;
    msg.to = to;
    msg.subject = subject;
    msg.body = body;
    msg.attachments = attachments;
    sendMail(msg);
}

void SmtpClient::clearQueue() {
    _queue.clear();
    if (_state != State::Idle) {
        resetSession();
    }
}

bool SmtpClient::isSending() const {
    return _state != State::Idle;
}

int SmtpClient::queueSize() const {
    return _queue.size();
}

void SmtpClient::processQueue() {
    if (_queue.isEmpty()) {
        changeState(State::Idle);
        emit allMailsProcessed();
        return;
    }

    // Не начинаем обработку, если сокет ещё подключен
    if (_socket->state() != QAbstractSocket::UnconnectedState) {
        qCDebug(logDebug) << "SmtpClient: ожидание отключения сокета";
        QTimer::singleShot(100, this, &SmtpClient::processQueue);
        return;
    }

    _currentMessage = _queue.dequeue();
    _tlsStarted = false;
    _authenticated = false;
    _responseBuffer.clear();

    qCDebug(logDebug) << "SmtpClient: подключение к" << _config.host << ":" << _config.port;
    emit stateChanged("Connecting...");

    changeState(State::WaitingForGreeting); // ← СНАЧАЛА меняем состояние

    if (_config.useSsl) {
        _socket->connectToHostEncrypted(_config.host, _config.port);
    } else {
        _socket->connectToHost(_config.host, _config.port);
    }

    _timeoutTimer->start(_config.timeoutMs); // ← ПОТОМ запускаем таймер
}

void SmtpClient::onSocketConnected() {
    qCDebug(logDebug) << "SmtpClient: подключено";
    _timeoutTimer->start(_config.timeoutMs);
}

void SmtpClient::onSocketDisconnected() {
    qCDebug(logDebug) << "SmtpClient: отключено";
    _timeoutTimer->stop();

    if (_state != State::Idle && _state != State::WaitingForQuitResponse) {
        failCurrentMail("Соединение разорвано");
    }

    resetSession();
    processQueue();
}

void SmtpClient::onSocketError(QAbstractSocket::SocketError error) {
    // Сервер закрыл соединение в ответ на команду QUIT — это штатное завершение
    if ((_state == State::WaitingForQuitResponse || _state == State::SendingQuit)
        && error == QAbstractSocket::RemoteHostClosedError) {
        qCDebug(logDebug) << "SmtpClient: сервер закрыл соединение после QUIT";
        return;
    }

    QString errorMsg = _socket->errorString();
    qCDebug(logDebug) << "SmtpClient: ошибка сокета:" << errorMsg;

    if (_state != State::Idle) {
        failCurrentMail("Ошибка сокета: " + errorMsg);
    }

    bool wasUnconnected = (_socket->state() == QAbstractSocket::UnconnectedState);
    resetSession();
    if (wasUnconnected) {
        processQueue();
    }
}


void SmtpClient::onSslErrors(const QList<QSslError> &errors) {
    QStringList errorStrings;
    for (const auto &err : errors) {
        errorStrings << err.errorString();
    }
    qCDebug(logDebug) << "SmtpClient: SSL ошибки:" << errorStrings.join(", ");
    _socket->ignoreSslErrors();
}

void SmtpClient::onSocketEncrypted() {
    qCDebug(logDebug) << "SmtpClient: TLS соединение установлено";
    if (_tlsStarted && _state == State::WaitingForStartTlsResponse) {
        changeState(State::WaitingForEhloResponse);
        sendCommand("EHLO localhost");
    }
}

void SmtpClient::onSocketReadyRead() {
    _responseBuffer += _socket->readAll();

    if (!_responseBuffer.endsWith("\r\n")) {
        return;
    }

    // Ищем предпоследний \r\n, чтобы выделить последнюю строку целиком (без завершающих \r\n)
    int prevNewline = _responseBuffer.lastIndexOf("\r\n", _responseBuffer.size() - 3);
    QByteArray lastLine = (prevNewline == -1)
                              ? _responseBuffer.left(_responseBuffer.size() - 2)
                              : _responseBuffer.mid(prevNewline + 2, _responseBuffer.size() - prevNewline - 4);

    if (lastLine.size() < 4 || lastLine.at(3) != ' ') {
        return; // Многострочный ответ (например, "250-localhost\r\n") ещё не закончен
    }

    QByteArray response = _responseBuffer;
    _responseBuffer.clear();

    qCDebug(logDebug) << "SmtpClient: получен ответ:" << response.trimmed();

    int code = extractResponseCode(lastLine);

    switch (_state) {
    case State::WaitingForGreeting:
        if (code == 220) {
            changeState(State::WaitingForEhloResponse);
            sendCommand("EHLO localhost");
        } else {
            failCurrentMail("Неверный greeting: " + response);
        }
        break;

    case State::WaitingForEhloResponse:
        if (code == 250) {
            if (_config.useStartTls && !_tlsStarted) {
                changeState(State::WaitingForStartTlsResponse);
                sendCommand("STARTTLS");
            } else if (!_config.username.isEmpty()) {
                changeState(State::WaitingForAuthResponse);
                sendCommand("AUTH LOGIN");
            } else {
                changeState(State::WaitingForMailFromResponse);
                sendCommand(("MAIL FROM: <" + _config.from + ">").toUtf8());
            }
        } else {
            failCurrentMail("EHLO failed: " + response);
        }
        break;

    case State::WaitingForStartTlsResponse:
        if (code == 220) {
            _tlsStarted = true;
            qCDebug(logDebug) << "SmtpClient: запуск TLS";
            _socket->startClientEncryption();
            _timeoutTimer->start(_config.timeoutMs);
        } else {
            failCurrentMail("STARTTLS failed: " + response);
        }
        break;

    case State::WaitingForAuthResponse:
        if (code == 334) {
            if (!_authenticated) {
                sendRawData(encodeBase64(_config.username.toUtf8()) + "\r\n");
                _authenticated = true;
            } else {
                sendRawData(encodeBase64(_config.password.toUtf8()) + "\r\n");
            }
            _timeoutTimer->start(_config.timeoutMs);
        } else if (code == 235) {
            qCDebug(logDebug) << "SmtpClient: авторизация успешна";
            changeState(State::WaitingForMailFromResponse);
            sendCommand(("MAIL FROM: <" + _config.from + ">").toUtf8());
        } else {
            failCurrentMail("AUTH failed: " + response);
        }
        break;

    case State::WaitingForMailFromResponse:
        if (code == 250) {
            changeState(State::WaitingForRcptToResponse);
            sendCommand(("RCPT TO: <" + _currentMessage.to + ">").toUtf8());
        } else {
            failCurrentMail("MAIL FROM failed: " + response);
        }
        break;

    case State::WaitingForRcptToResponse:
        if (code == 250) {
            changeState(State::WaitingForDataResponse);
            sendCommand("DATA");
        } else {
            failCurrentMail("RCPT TO failed: " + response);
        }
        break;

    case State::WaitingForDataResponse:
        if (code == 354) {
            changeState(State::WaitingForMessageResponse);
            sendRawData(buildMessage());
            sendCommand(".");
        } else {
            failCurrentMail("DATA failed: " + response);
        }
        break;

    case State::WaitingForMessageResponse:
        if (code == 250) {
            succeedCurrentMail();
            changeState(State::WaitingForQuitResponse);
            sendCommand("QUIT");
        } else {
            failCurrentMail("Message sending failed: " + response);
        }
        break;

    case State::WaitingForQuitResponse:
        _socket->disconnectFromHost();
        break;

    default:
        break;
    }
}

void SmtpClient::onTimeout() {
    qCDebug(logDebug) << "SmtpClient: таймаут в состоянии" << static_cast<int>(_state);

    if (_state != State::Idle) {
        failCurrentMail("Таймаут операции");
    }

    resetSession();
    if (_socket->state() == QAbstractSocket::UnconnectedState) {
        processQueue();
    }
}

void SmtpClient::sendCommand(const QByteArray &command) {
    qCDebug(logDebug) << "SmtpClient: отправка команды:" << command;
    _socket->write(command + "\r\n");
    _timeoutTimer->start(_config.timeoutMs);
}

void SmtpClient::sendRawData(const QByteArray &data) {
    _socket->write(data);
    _timeoutTimer->start(_config.timeoutMs);
}

void SmtpClient::changeState(State newState) {
    _state = newState;

    QString stateStr;
    switch (newState) {
    case State::Idle: stateStr = "Idle"; break;
    case State::WaitingForGreeting: stateStr = "Waiting for greeting"; break;
    case State::SendingEhlo:
    case State::WaitingForEhloResponse: stateStr = "EHLO"; break;
    case State::SendingStartTls:
    case State::WaitingForStartTlsResponse: stateStr = "STARTTLS"; break;
    case State::SendingAuth:
    case State::WaitingForAuthResponse: stateStr = "AUTH"; break;
    case State::SendingMailFrom:
    case State::WaitingForMailFromResponse: stateStr = "MAIL FROM"; break;
    case State::SendingRcptTo:
    case State::WaitingForRcptToResponse: stateStr = "RCPT TO"; break;
    case State::SendingData:
    case State::WaitingForDataResponse: stateStr = "DATA"; break;
    case State::SendingMessage:
    case State::WaitingForMessageResponse: stateStr = "Sending message"; break;
    case State::SendingQuit:
    case State::WaitingForQuitResponse: stateStr = "QUIT"; break;
    }

    emit stateChanged(stateStr);
}

void SmtpClient::resetSession() {
    _timeoutTimer->stop();
    _state = State::Idle;
    _tlsStarted = false;
    _authenticated = false;
    _responseBuffer.clear();

    if (_socket->state() != QAbstractSocket::UnconnectedState) {
        _socket->abort(); // Мгновенно закрывает сокет и вызывает disconnected
    }
}

void SmtpClient::failCurrentMail(const QString &errorMsg) {
    qCDebug(logDebug) << "SmtpClient: ошибка отправки письма:" << errorMsg;
    emit mailFailed(_currentMessage.to, errorMsg);
    resetSession();
}

void SmtpClient::succeedCurrentMail() {
    qCDebug(logDebug) << "SmtpClient: письмо успешно отправлено:" << _currentMessage.to;
    emit mailSent(_currentMessage.to);
}

QByteArray SmtpClient::buildMessage() const {
    QByteArray message;
    QString boundary = "----=_Part_" + QString::number(QRandomGenerator::global()->generate());

    QByteArray subjectBase64 = _currentMessage.subject.toUtf8().toBase64();

    message += "From: " + _config.from.toUtf8() + "\r\n";
    message += "To: " + _currentMessage.to.toUtf8() + "\r\n";
    //message += "Subject: " + _currentMessage.subject.toUtf8() + "\r\n";
    message += "Subject: =?UTF-8?B?" + subjectBase64 + "?=\r\n";
    message += "Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8() + "\r\n";
    message += "MIME-Version: 1.0\r\n";

    if (_currentMessage.attachments.isEmpty()) {
        if (_currentMessage.isHtml) {
            message += "Content-Type: text/html; charset=\"utf-8\"\r";
        } else {
            message += "Content-Type: text/plain; charset=\"utf-8\"\r";
        }
        message += "Content-Transfer-Encoding: 8bit\r\n\r\n";
        message += _currentMessage.body.toUtf8() + "\r\n";
    } else {
        message += "Content-Type: multipart/mixed; boundary=\"" + boundary.toUtf8() + "\"\r\n\r\n";

        message += "--" + boundary.toUtf8() + "\r\n";
        if (_currentMessage.isHtml) {
            message += "Content-Type: text/html; charset=\"utf-8\"\r";
        } else {
            message += "Content-Type: text/plain; charset=\"utf-8\"\r";
        }
        message += "Content-Transfer-Encoding: 8bit\r\n\r\n";
        message += _currentMessage.body.toUtf8() + "\r\n";

        for (const QString &filePath : _currentMessage.attachments) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                qCDebug(logDebug) << "SmtpClient: не удалось открыть файл:" << filePath;
                continue;
            }

            QByteArray fileData = file.readAll();
            file.close();

            QString fileName = extractFileName(filePath);
            QByteArray encodedFileName = "=?UTF-8?B?" + fileName.toUtf8().toBase64() + "?=";
            QByteArray urlEncodedFileName = QUrl::toPercentEncoding(fileName);

            message += "--" + boundary.toUtf8() + "\r\n";
            message += "Content-Type: application/octet-stream; name=\"" + encodedFileName + "\"\r\n";
            message += "Content-Transfer-Encoding: base64\r\n";
            message += "Content-Disposition: attachment; filename=\"" + encodedFileName + "\"; filename*=UTF-8''" + urlEncodedFileName + "\r\n\r\n";

            QByteArray encoded = fileData.toBase64();
            for (int i = 0; i < encoded.size(); i += 76) {
                message += encoded.mid(i, 76) + "\r\n";
            }
        }

        message += "--" + boundary.toUtf8() + "--\r\n";
    }

    return message;
}

QByteArray SmtpClient::encodeBase64(const QByteArray &data) const {
    return data.toBase64();
}

QString SmtpClient::extractFileName(const QString &filePath) const {
    QFileInfo fi(filePath);
    return fi.fileName();
}

bool SmtpClient::verifyResponseCode(const QByteArray &response, int expectedCode) const {
    int code = extractResponseCode(response);
    return code == expectedCode;
}

int SmtpClient::extractResponseCode(const QByteArray &response) const {
    if (response.size() < 3) return -1;
    return response.left(3).toInt();
}
