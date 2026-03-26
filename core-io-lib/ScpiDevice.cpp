#include "ScpiDevice.h"

#include "Logger.h"

// ScpiDevice::ScpiDevice(const QString &ip, unsigned int port, QObject *parent) :
//     QObject(parent)
// {
//     setIpAddr(ip);
//     setPort(port);

//     _socket = std::unique_ptr<QTcpSocket,decltype(_qobj_deleter)> (new QTcpSocket(nullptr), _qobj_deleter);
//     _timeout_timer = std::unique_ptr<QTimer,decltype(_qobj_deleter)> (new QTimer(nullptr),_qobj_deleter);
//     _local_event_loop = std::unique_ptr<QEventLoop,decltype(_qobj_deleter)> (new QEventLoop(nullptr), _qobj_deleter);

//     // We don't need a cyclic timer
//     _timeout_timer->setSingleShot(true);

//     connect(this, &ScpiDevice::needReconnect, this, &ScpiDevice::startReconnect, Qt::QueuedConnection);

//     QObject::connect(_socket.get(), &QTcpSocket::connected, this, &ScpiDevice::onConnected);
//     QObject::connect(_socket.get(), &QTcpSocket::disconnected, this, &ScpiDevice::onDisconnected);
//     QObject::connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ScpiDevice::onSocketError);

//     _reconnect_timer = new QTimer(this);
//     _reconnect_timer->setSingleShot(true);
//     QObject::connect(_reconnect_timer, &QTimer::timeout, this, &ScpiDevice::startReconnect);
// }

ScpiDevice::ScpiDevice(const QString &ip, unsigned int port, QObject *parent)
    : QObject(parent)
{
    m_transport = std::make_unique<TcpTransport>(ip, port, this);
    initTransport();
}

ScpiDevice::ScpiDevice(const QString &portName,
                       qint32 baudRate,
                       QSerialPort::DataBits dataBits,
                       QSerialPort::StopBits stopBits,
                       QSerialPort::Parity parity,
                       QObject *parent)
    : QObject(parent)
{
    m_transport = std::make_unique<SerialTransport>(portName, baudRate, dataBits, stopBits, parity, this);
    initTransport();
}

ScpiDevice::~ScpiDevice() {
    disconnectFromDevice();
}

void ScpiDevice::initTransport()
{
    // Set initial timeout
    m_transport->setTimeout(_timeout_ms);

    // Connect transport signals to ScpiDevice slots
    connect(m_transport.get(), &ScpiTransport::connected,
            this, &ScpiDevice::onConnected);
    connect(m_transport.get(), &ScpiTransport::disconnected,
            this, &ScpiDevice::onDisconnected);
    connect(m_transport.get(), &ScpiTransport::errorOccurred,
            this, &ScpiDevice::onTransportError);

    // Reconnection timer
    _reconnect_timer = new QTimer(this);
    _reconnect_timer->setSingleShot(true);
    connect(_reconnect_timer, &QTimer::timeout, this, &ScpiDevice::startReconnect);

    // Connect reconnection request (internal)
    connect(this, &ScpiDevice::needReconnect, this, &ScpiDevice::startReconnect, Qt::QueuedConnection);
}

void ScpiDevice::connectRequest() {
    this->connectToDevice();
}

void ScpiDevice::disconnectRequest() {
    this->disconnectFromDevice();
}

void ScpiDevice::connectToDevice() {
    // _reconnect_timer->stop();

    // _is_reconnecting = false;
    // disconnectFromDevice();

    // QObject::connect(_socket.get(), &QTcpSocket::connected, _local_event_loop.get(), &QEventLoop::quit);
    // QObject::connect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

    // emit connecting();

    // _socket->connectToHost(_ip,_port);
    // _timeout_timer->start(_timeout_ms);
    // _local_event_loop->exec();

    // bool timeout_occurred = !_timeout_timer->isActive();
    // _timeout_timer->stop();

    // QObject::disconnect(_socket.get(), &QTcpSocket::bytesWritten, _local_event_loop.get(), &QEventLoop::quit);
    // QObject::disconnect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

    // if (timeout_occurred) {
    //     //throw ScpiDeviceError("Error connecting to a device. Timeout expired.");
    //     qCDebug(logDebug) << "Error connecting to a device. Timeout expired.";
    //     this->tryReconnect();
    // }

    // return;

    _reconnect_timer->stop();
    _is_reconnecting = false;

    // Disconnect first to ensure clean state
    disconnectFromDevice();

    emit connecting();
    m_transport->connectToDevice();
}

void ScpiDevice::disconnectFromDevice() noexcept {
    // _reconnect_timer->stop();

    // if (!_socket->isValid())
    //     return;

    // if (_socket->state() == QAbstractSocket::UnconnectedState)
    //     return;

    // _socket->disconnectFromHost();

    _reconnect_timer->stop();
    m_transport->disconnectFromDevice();
}

void ScpiDevice::reset() {
    try {
        sendCommand("*RST;");
    }  catch (ScpiQueryError &err) {
        throw ScpiQueryError(QString("Unable to reset the device: %1").arg(err.what()));
    }
}

void ScpiDevice::clearState() {
    try {
        sendCommand("*CLS;");
    }  catch (ScpiQueryError &err) {
        throw ScpiQueryError(QString("Unable to clear the device's state register: %1").arg(err.what()));
    }
}

bool ScpiDevice::is_connected() noexcept {
    // if (_socket->isValid() && _socket->state() == QTcpSocket::ConnectedState)
    //     return true;
    // else
    //     return false;
    return m_transport && m_transport->isConnected();
}

QString ScpiDevice::getIDN() {
    try {
        return sendQuery("*IDN?");
    }  catch (ScpiQueryError &err) {
        throw ScpiQueryError(QString("Unable to get an ID string: %1").arg(err.what()));
    }
}

//void ScpiDevice::sendCommand(const QString &command) {
//    /* The command gets extended with the request of its completion (OPerationComplete?)
//    and sended as a query (it's fine). A device will return 1 only after the command
//    will be actually executed. */
//    QString status = sendQuery(QString("%1; *OPC?").arg(command));
//    if( status.toInt() != 1 )
//        throw ScpiQueryError("The device has returned an unexpected OPC code (not 1).");
//}

//QString ScpiDevice::sendQuery(const QString &query) {
//    if (!socket->isValid() || socket->state() != QTcpSocket::ConnectedState)
//        throw ScpiDeviceError("Incorrect socket state (invalid or not connected).");

//    // Extend a query with the terminating symbol
//    QString query_eol = QString("%1\n").arg(query);

//    QObject::connect(socket.get(), &QTcpSocket::bytesWritten, localEventloop.get(), &QEventLoop::quit);
//    QObject::connect(timeoutTimer.get(), &QTimer::timeout, localEventloop.get(), &QEventLoop::quit);

//    // Send a query, wait for the sending completion
//    socket->write(query_eol.toLocal8Bit());
//    timeoutTimer->start(_timeout_ms);
//    localEventloop->exec();

//    if(!timeoutTimer->isActive())
//        throw ScpiQueryError("Sending timeout expired.");


//    QObject::disconnect(socket.get(), &QTcpSocket::bytesWritten, localEventloop.get(), &QEventLoop::quit);
//    QObject::connect(socket.get(), &QTcpSocket::readyRead, localEventloop.get(), &QEventLoop::quit);

//    // Wait for the device ready to send a response
//    timeoutTimer->start(_timeout_ms);
//    localEventloop->exec();

//    if(!timeoutTimer->isActive())
//        throw ScpiQueryError("Receiving timeout expired.");

//    // Принимаем ответ
//    return QString(socket->readAll());
//}

//void ScpiDevice::sendCommand(const QString &command) {
//    if (!_socket->isValid() || _socket->state() != QTcpSocket::ConnectedState)
//        throw ScpiDeviceError("Incorrect socket state (invalid or not connected).");

//    qCDebug(logDebug) << command;

//    // Extend a query with the terminating symbol
//    QString command_eol = QString("%1\r\n").arg(command);

//    QObject::connect(_socket.get(), &QTcpSocket::bytesWritten, _local_event_loop.get(), &QEventLoop::quit);
//    QObject::connect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

//    // Send a query, wait for the sending completion
//    _socket->write(command_eol.toLocal8Bit());
//    _timeout_timer->start(_timeout_ms);
//    _local_event_loop->exec();

//    if(!_timeout_timer->isActive())
//        throw ScpiQueryError("Sending timeout expired.");

//    QObject::disconnect(_socket.get(), &QTcpSocket::bytesWritten, _local_event_loop.get(), &QEventLoop::quit);
//}

void ScpiDevice::sendCommand(const QString &command) {
    try {
        //if (_is_reconnecting || !_socket->isValid() || _socket->state() != QTcpSocket::ConnectedState)
        if (_is_reconnecting || !is_connected())
            throw ScpiDeviceError("Device is not connected.");

        /* The command gets extended with the request of its completion (OPerationComplete?)
        and sended as a query (it's fine). A device will return 1 only after the command
        will be actually executed. */

        auto status = sendQuery(QString("%1; *OPC?").arg(command));
        if (status.toInt() != 1)
            throw ScpiQueryError("The device has returned an unexpected OPC code (not 1).");
    } catch (const ScpiDeviceError &err) {
        emit needReconnect();
        throw;
    }
}

QByteArray ScpiDevice::sendQuery(const QString &query) {
    // try {
    //     if (_is_reconnecting || !_socket->isValid() || _socket->state() != QTcpSocket::ConnectedState)
    //         throw ScpiDeviceError("Device is not connected.");

    //     qCDebug(logDebug) << query;

    //     // Очищаем буфер перед отправкой
    //     _socket->readAll();

    //     // Extend a query with the terminating symbol
    //     QString query_eol = QString("%1\r\n").arg(query);

    //     QObject::connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::connect(_socket.get(), &QTcpSocket::bytesWritten, _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::connect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

    //     // Send a query, wait for the sending completion
    //     _socket->write(query_eol.toLocal8Bit());
    //     _timeout_timer->start(_timeout_ms);
    //     _local_event_loop->exec();

    //     bool timeout_occurred = !_timeout_timer->isActive();
    //     _timeout_timer->stop();

    //     QObject::disconnect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::disconnect(_socket.get(), &QTcpSocket::bytesWritten, _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::disconnect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

    //     if(timeout_occurred)
    //         throw ScpiQueryError("Sending timeout expired.");

    //     QObject::connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::connect(_socket.get(), &QTcpSocket::readyRead, _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::connect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

    //     // QObject::disconnect(_socket.get(), &QTcpSocket::bytesWritten, _local_event_loop.get(), &QEventLoop::quit);
    //     // QObject::connect(_socket.get(), &QTcpSocket::readyRead, _local_event_loop.get(), &QEventLoop::quit);

    //     // Wait for the device ready to send a response
    //     _timeout_timer->start(_timeout_ms);
    //     _local_event_loop->exec();

    //     timeout_occurred = !_timeout_timer->isActive();
    //     _timeout_timer->stop();

    //     QObject::disconnect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::disconnect(_socket.get(), &QTcpSocket::readyRead, _local_event_loop.get(), &QEventLoop::quit);
    //     QObject::disconnect(_timeout_timer.get(), &QTimer::timeout, _local_event_loop.get(), &QEventLoop::quit);

    //     if(timeout_occurred)
    //         throw ScpiQueryError("Receiving timeout expired.");

    //     // Принимаем ответ
    //     auto res = _socket->readAll();
    //     if (res.size() < 100) {
    //         qCDebug(logDebug) << res;
    //     }

    //     return res;
    // } catch (const ScpiDeviceError &err) {
    //     emit needReconnect();
    //     throw;
    // }

    // return {};

    if (_is_reconnecting || !is_connected())
        throw ScpiDeviceError("Device is not connected.");

    qCDebug(logDebug) << query;

    try {
        QByteArray result = m_transport->sendQuery(query.toLocal8Bit());
        if (result.size() < 100) {
            qCDebug(logDebug) << result;
        }
        return result;
    } catch (const std::exception &e) {
        // The transport already emitted errorOccurred, which will trigger reconnect.
        // But we also need to propagate exception upward.
        throw ScpiQueryError(QString("Query failed: %1").arg(e.what()));
    }
}

void ScpiDevice::setTimeout(unsigned int scpi_timeout) {
    // It's risky to set a big timeout. The program may hang, waiting for it.
    // Usually 2-3 seconds are more than enough.
    if (scpi_timeout> 100000)
        throw ScpiDeviceError("The SCPI timeout is too large.");
    else
        _timeout_ms = scpi_timeout;
}

void ScpiDevice::tryReconnect() {
    qCWarning(logDebug) << "Attempting to reconnect...";
    try {
        disconnectFromDevice();
        connectToDevice();
    } catch (const std::exception& e) {
        qCDebug(logDebug) << "Reconnect failed:" << e.what();
        // Таймер не останавливаем — будет повторять попытки
    }
}

void ScpiDevice::onConnected()
{
    qCDebug(logDebug) << "ScpiDevice connected";
    _is_reconnecting = false;
    emit reconnectingFinished();
    emit connected();
}

void ScpiDevice::onDisconnected()
{
    qCWarning(logDebug) << "ScpiDevice disconnected";
    if (!_is_reconnecting) {
        _is_reconnecting = true;
        _reconnect_timer->start(_timeout_ms);
    }
    emit disconnected();
}

void ScpiDevice::onTransportError(const QString &errorString)
{
    qCWarning(logDebug) << "Transport error:" << errorString;
    if (!_is_reconnecting && !is_connected()) {
        _is_reconnecting = true;
        _reconnect_timer->start(_timeout_ms);
    }
}

// void ScpiDevice::onSocketError(QAbstractSocket::SocketError socket_error) {
//     Q_UNUSED(socket_error)
//     qCWarning(logDebug) << "Socket error:" << _socket->errorString();
//     if (_socket->state() == QAbstractSocket::UnconnectedState && !_is_reconnecting) {
//         _is_reconnecting = true;
//         _reconnect_timer->start(_timeout_ms);
//     }
// }

// void ScpiDevice::recreateSocket() {
//     // Отключаем старый сокет, если он существует
//     if (_socket) {
//         _socket->abort();
//         _socket->disconnectFromHost();
//         _socket->deleteLater();
//         _socket.reset();
//     }

//     // Создаём новый QTcpSocket
//     _socket = std::unique_ptr<QTcpSocket, decltype(_qobj_deleter)>(new QTcpSocket(nullptr), _qobj_deleter);

//     // Переподключаем сигналы
//     QObject::connect(_socket.get(), &QTcpSocket::connected, this, &ScpiDevice::connected);
//     QObject::connect(_socket.get(), &QTcpSocket::disconnected, this, &ScpiDevice::onDisconnected);
//     QObject::connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ScpiDevice::onSocketError);
// }

void ScpiDevice::startReconnect() {
    if (!_is_reconnecting) {
        _is_reconnecting = true;
        emit reconnectingStarted();
        tryReconnect();
    }
}

