#include "ScpiTransport.h"

#include <QAbstractSocket>
#include <QSerialPortInfo>

TcpTransport::TcpTransport(const QString &host, quint16 port, QObject *parent)
    : ScpiTransport(parent)
    , m_socket(new QTcpSocket(this))
    , m_timeoutTimer(new QTimer(this))
    , m_eventLoop(new QEventLoop(this))
    , m_host(host)
    , m_port(port)
    , m_timeoutMs(5000)
{
    m_timeoutTimer->setSingleShot(true);

    connect(m_socket, &QTcpSocket::connected, this, &TcpTransport::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpTransport::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &TcpTransport::onSocketError);
}

TcpTransport::~TcpTransport()
{
    disconnectFromDevice();
}

void TcpTransport::connectToDevice()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        return;

    disconnectFromDevice();

    // Подключаем сигналы для ожидания
    connect(m_socket, &QTcpSocket::connected, m_eventLoop, &QEventLoop::quit);
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_socket->connectToHost(m_host, m_port);
    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    bool timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_socket, &QTcpSocket::connected, m_eventLoop, &QEventLoop::quit);
    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    if (timeout) {
        emit errorOccurred("Connection timeout");
    }
}

void TcpTransport::disconnectFromDevice()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        // Ждем отключения? Можно не ждать, так как событие придет асинхронно.
    }
}

bool TcpTransport::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpTransport::sendCommand(const QByteArray &command)
{
    if (!isConnected()) {
        emit errorOccurred("Not connected");
        return;
    }

    QByteArray cmd = command + "\r\n";

    connect(m_socket, &QTcpSocket::bytesWritten, m_eventLoop, &QEventLoop::quit);
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_socket->write(cmd);
    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    bool timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_socket, &QTcpSocket::bytesWritten, m_eventLoop, &QEventLoop::quit);
    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    if (timeout) {
        emit errorOccurred("Send timeout");
    }
}

QByteArray TcpTransport::sendQuery(const QByteArray &query)
{
    if (!isConnected()) {
        emit errorOccurred("Not connected");
        return QByteArray();
    }

    // Очищаем буфер перед отправкой
    m_socket->readAll();

    QByteArray cmd = query + "\r\n";

    // Этап отправки
    connect(m_socket, &QTcpSocket::bytesWritten, m_eventLoop, &QEventLoop::quit);
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_socket->write(cmd);
    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    bool timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_socket, &QTcpSocket::bytesWritten, m_eventLoop, &QEventLoop::quit);
    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    if (timeout) {
        emit errorOccurred("Send timeout");
        return QByteArray();
    }

    // Этап получения ответа
    connect(m_socket, &QTcpSocket::readyRead, m_eventLoop, &QEventLoop::quit);
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_socket, &QTcpSocket::readyRead, m_eventLoop, &QEventLoop::quit);
    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    if (timeout) {
        emit errorOccurred("Receive timeout");
        return QByteArray();
    }

    return m_socket->readAll();
}

void TcpTransport::onConnected()
{
    emit connected();
}

void TcpTransport::onDisconnected()
{
    emit disconnected();
}

void TcpTransport::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    emit errorOccurred(m_socket->errorString());
}




SerialTransport::SerialTransport(const QString &portName,
                                 qint32 baudRate,
                                 QSerialPort::DataBits dataBits,
                                 QSerialPort::StopBits stopBits,
                                 QSerialPort::Parity parity,
                                 QObject *parent)
    : ScpiTransport(parent)
    , m_serial(new QSerialPort(this))
    , m_timeoutTimer(new QTimer(this))
    , m_eventLoop(new QEventLoop(this))
    , m_currentEventLoop(nullptr)
    , m_portName(portName)
    , m_baudRate(baudRate)
    , m_dataBits(dataBits)
    , m_stopBits(stopBits)
    , m_parity(parity)
    , m_timeoutMs(5000)
{
    m_timeoutTimer->setSingleShot(true);

    connect(m_serial, &QSerialPort::readyRead, this, &SerialTransport::onReadyRead);
    connect(m_serial, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &SerialTransport::onErrorOccurred);
}

SerialTransport::~SerialTransport()
{
    disconnectFromDevice();
}

void SerialTransport::connectToDevice()
{
    if (isConnected())
        return;

    disconnectFromDevice();

    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(m_baudRate);
    m_serial->setDataBits(m_dataBits);
    m_serial->setStopBits(m_stopBits);
    m_serial->setParity(m_parity);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serial->errorString());
        return;
    }

    emit connected();
}

void SerialTransport::disconnectFromDevice()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        emit disconnected();
    }
}

bool SerialTransport::isConnected() const
{
    return m_serial->isOpen();
}

void SerialTransport::sendCommand(const QByteArray &command)
{
    if (!isConnected()) {
        emit errorOccurred("Not connected");
        return;
    }

    QByteArray cmd = command + "\r\n";

    connect(m_serial, &QSerialPort::bytesWritten, m_eventLoop, &QEventLoop::quit);
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_serial->write(cmd);
    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    bool timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_serial, &QSerialPort::bytesWritten, m_eventLoop, &QEventLoop::quit);
    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    if (timeout) {
        emit errorOccurred("Send timeout");
    }
}

QByteArray SerialTransport::sendQuery(const QByteArray &query)
{
    if (!isConnected()) {
        emit errorOccurred("Not connected");
        return QByteArray();
    }

    // Clear buffers
    m_serial->readAll();
    m_readBuffer.clear();

    QByteArray cmd = query + "\r\n";

    // Send phase
    connect(m_serial, &QSerialPort::bytesWritten, m_eventLoop, &QEventLoop::quit);
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_serial->write(cmd);
    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    bool timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_serial, &QSerialPort::bytesWritten, m_eventLoop, &QEventLoop::quit);
    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    if (timeout) {
        emit errorOccurred("Send timeout");
        return QByteArray();
    }

    // Receive phase
    m_currentEventLoop = m_eventLoop;
    connect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);

    m_timeoutTimer->start(m_timeoutMs);
    m_eventLoop->exec();

    timeout = !m_timeoutTimer->isActive();
    m_timeoutTimer->stop();

    disconnect(m_timeoutTimer, &QTimer::timeout, m_eventLoop, &QEventLoop::quit);
    m_currentEventLoop = nullptr;

    if (timeout) {
        emit errorOccurred("Receive timeout");
        return QByteArray();
    }

    return m_readBuffer;
}

void SerialTransport::onReadyRead()
{
    m_readBuffer.append(m_serial->readAll());
    // Если накопилась полная строка (с \n), завершаем текущий цикл
    if (m_readBuffer.contains('\n')) {
        if (m_currentEventLoop) {
            m_currentEventLoop->quit();
        }
    }
}

void SerialTransport::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        emit errorOccurred(m_serial->errorString());
    }
}
