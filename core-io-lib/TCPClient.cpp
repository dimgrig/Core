#include "TCPClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef Q_OS_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include "Logger.h"

static std::chrono::steady_clock::time_point rx_st;
static std::chrono::steady_clock::time_point prev_rx_st;
static std::chrono::steady_clock::time_point tx_end;

TCPClient::TCPClient(QWidget *parent) :
        QWidget(parent)
{
    _socket->setSocketOption(QAbstractSocket::KeepAliveOption,1);
    _socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

//    connect(_socket,SIGNAL(readyRead()),SLOT(readSocketData()),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(error(QAbstractSocket::SocketError)),SIGNAL(connectionError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),SIGNAL(tcpSocketState(QAbstractSocket::SocketState)),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(disconnected()),SLOT(onConnectionTerminated()),Qt::UniqueConnection);
//    connect(_socket,SIGNAL(connected()),SLOT(onConnectionEstablished()),Qt::UniqueConnection);

    //connect(_socket, &QTcpSocket::readyRead, this, &TCPClient::readData);
#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
    connect(_socket, &QTcpSocket::errorOccurred, this, &TCPClient::handleError);
#endif
    connect(_socket, &QTcpSocket::connected, this, [=](){
        qCDebug(logInfo()) << "TCP: connected to" << this->_host << this->_port;
        emit host_connected();
        this->setSockOpt();
    });
    connect(_socket, &QTcpSocket::disconnected, this, [=](){
        qCDebug(logInfo()) << "TCP: disconnected from" << this->_host << this->_port;
        emit host_disconnected();
    });

    //no connect in ctor
//    if(!(QAbstractSocket::ConnectedState == _socket->state())){
//        _socket->connectToHost(cfg.IP, cfg.PORT, QIODevice::ReadWrite);
//    }
}

TCPClient::~TCPClient() {
    if (_socket && this->isConnected()) {
        this->disconnectFromHost();
    }
}

void TCPClient::set_async(bool async) {
    _async = async;
    if (async) {
        connect(_socket, &QTcpSocket::readyRead, this, &TCPClient::readDataSlot);
    } else {
        disconnect(_socket, &QTcpSocket::readyRead, this, &TCPClient::readDataSlot);
    }
}

void TCPClient::setSockOpt() {
//    //works after connection to host!!!
//    int fd = _socket->socketDescriptor();

//    int enableKeepAlive = 1;
//    int res = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
//    qCDebug(logDebug) << "setsockopt SO_KEEPALIVE " << res;

////    int maxIdle = 60; /* seconds */
////    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));

////    int count = 3;  // send up to 3 keepalive packets out, then disconnect if no response
////    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));

////    int interval = 2;   // send a keepalive packet out every 2 seconds (after the 5 second idle period)
////    setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));

//    int enableNoDelay = 1;
//    res = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enableNoDelay, sizeof(enableNoDelay));
//    qCDebug(logDebug) << "setsockopt TCP_NODELAY " << res;

//    //res = setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, (int[]){1}, sizeof(int));
//    //qCDebug(logDebug) << "setsockopt TCP_QUICKACK " << res;
//    int enableQuickACK = 1;
//    res = setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &enableQuickACK, sizeof(enableQuickACK));
//    qCDebug(logDebug) << "setsockopt TCP_QUICKACK " << res;
}

#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
void TCPClient::handleError(QAbstractSocket::SocketError socketError){
    _socket->disconnectFromHost();
    if (_socket->state() == QAbstractSocket::UnconnectedState || _socket->waitForDisconnected(1000)) {

    }

    switch(socketError){
        case QAbstractSocket::RemoteHostClosedError:
            qCDebug(logWarning()) << "TCP: RemoteHostClosedError";
        break;
        case QAbstractSocket::HostNotFoundError:
            qCDebug(logWarning()) << "TCP: HostNotFoundError. Connection was refused. Server is not working";
        break;
        case QAbstractSocket::ConnectionRefusedError:
            qCDebug(logWarning()) << "TCP: ConnectionRefusedError. Client. The host was not found";
        break;
        default:
            qCDebug(logWarning()) << "TCP: Error" << _socket->errorString();
        break;
    }

//    if(!(QAbstractSocket::ConnectedState == _socket->state())){
//        //_socket->connectToHost(cfg.IP, cfg.PORT, QIODevice::ReadWrite);
//        QTimer::singleShot(1000, this, [=](){
//            _socket->connectToHost(cfg.IP, cfg.PORT, QIODevice::ReadWrite);
//        });
//    }
    //_socket->reset();
}
#endif

void TCPClient::connectToHost(QString host, int port) {
    _socket->connectToHost(host, port, QIODevice::ReadWrite);
    this->_host = host;
    this->_port = port;
}

void TCPClient::disconnectFromHost() {
    if (this->isConnected()) {
        _socket->disconnectFromHost();
        //timer->stop();
    }
}

void TCPClient::writeData(const char *buffer_tx, uint16_t length){
    if (_socket->isOpen()) {
        _socket->write(buffer_tx, length);
        _socket->flush();

#ifdef DEBUG_MSG_INTERFACES
            QByteArray txBuff = QByteArray::fromRawData(buffer_tx, length);
            qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
#endif
#ifdef PROFILER
        tx_end = std::chrono::steady_clock::now();
        std::cout << "rx-tx " << std::chrono::duration_cast<std::chrono::microseconds>(tx_end - rx_st).count() << "mcs" << std::endl;
#endif
    }
}

void TCPClient::writeData(const QByteArray &txBuff){
//    QByteArray l_vDataToBeSent;
//    QDataStream l_vStream(&l_vDataToBeSent, QIODevice::WriteOnly);
//    l_vStream.setByteOrder(QDataStream::LittleEndian);
//    l_vStream << txBuff.length();
//    l_vDataToBeSent.append(txBuff);

//    _socket->write(l_vDataToBeSent, l_vDataToBeSent.length());
//    _socket->flush();

//    if (cfg.DEBUG_MSG_ETHERNET) {
//        qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
//    }
    _socket->write(txBuff.data(), txBuff.length());
    _socket->flush();

#ifdef DEBUG_MSG_INTERFACES
        qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
#endif
#ifdef PROFILER
    tx_end = std::chrono::steady_clock::now();
    std::cout << "rx-tx " << std::chrono::duration_cast<std::chrono::microseconds>(tx_end - rx_st).count() << "mcs" << std::endl;
#endif
//    if (_socket) {
//        if (_socket->isOpen()) {
//            QDataStream socketStream(_socket);
//            //socketStream.setVersion(QDataStream::Qt_5_15);
//            socketStream << txBuff;
//            qCDebug(logDebug()) << Qt::hex << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
//        } else {
//            qCDebug(logWarning()) << "TCP: Socket doesn't seem to be opened";
//        }
//    } else {
//        qCDebug(logWarning()) << "TCP: Not connected";
//    }
}

std::string TCPClient::readData() {
    QByteArray rxBuff;
    while(_socket->bytesAvailable()){
        rxBuff += _socket->readAll();

#ifdef DEBUG_MSG_INTERFACES
            qCDebug(logDebug()) << QT_HEX << "TCP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif
    }

    return rxBuff.toStdString();
}

void TCPClient::readDataSlot(){
#ifdef PROFILER
    prev_rx_st = rx_st;
    rx_st = std::chrono::steady_clock::now();
    std::cout << "rx-rx " << std::chrono::duration_cast<std::chrono::microseconds>(rx_st - prev_rx_st).count() << "mcs" << std::endl;
#endif

    QByteArray rxBuff;
    while(_socket->bytesAvailable()){
        rxBuff += _socket->readAll();

#ifdef DEBUG_MSG_INTERFACES
            qCDebug(logDebug()) << QT_HEX << "TCP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif
    }

    emit read_data(std::move(rxBuff));
//    QByteArray rxBuff;

//    QDataStream socketStream(_socket);
//    //socketStream.setVersion(QDataStream::Qt_5_15);

//    socketStream.startTransaction();
//    socketStream >> rxBuff;

//    while (!socketStream.commitTransaction()) {
//        socketStream >> rxBuff;
//    }
//    qCDebug(logDebug()) << Qt::hex << "TCP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
//    emit read_data(rxBuff);
}
