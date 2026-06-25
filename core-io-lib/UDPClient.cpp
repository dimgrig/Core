#include "UDPClient.h"

#include "ClientHelpers.h"
#include "Logger.h"

#include "Categories.h"
#include "JournalAccess.h"

UDPClient::UDPClient(QWidget *parent) :
        QWidget(parent)
{
#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
    connect(_socket, &QAbstractSocket::errorOccurred, this, &UDPClient::handleError);
#endif
    connect(_socket, &QAbstractSocket::connected, this, [=](){
        qCDebug(logInfo()) << "UDP: connected to" << this->_host << this->_port;
        emit host_connected();
        this->setSockOpt();
    });
    connect(_socket, &QAbstractSocket::disconnected, this, [=](){
        qCDebug(logInfo()) << "UDP: disconnected from" << this->_host << this->_port;
        emit host_disconnected();
    });
}

UDPClient::~UDPClient() {
    if (_socket && this->isConnected()) {
        this->disconnectFromHost();
    }
}

void UDPClient::setSockOpt() {

}

#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
void UDPClient::handleError(QAbstractSocket::SocketError socketError){
    _socket->disconnectFromHost();
    if (_socket->state() == QAbstractSocket::UnconnectedState || _socket->waitForDisconnected(1000)) {

    }

    QString errorStr;

    switch(socketError){
        case QAbstractSocket::RemoteHostClosedError:
            errorStr = "RemoteHostClosedError";
        break;
        case QAbstractSocket::HostNotFoundError:
            errorStr = "HostNotFoundError. Connection was refused. Server is not working";
        break;
        case QAbstractSocket::ConnectionRefusedError:
            errorStr = "ConnectionRefusedError. Client. The host was not found";
        break;
        default:
            errorStr = _socket->errorString();
        break;
    }

    if (auto* j = JournalAccess::journal())
        j->warning(Category::IO, QString("UDP: ошибка %1").arg(errorStr));
}
#endif

void UDPClient::connectToHost(QString host, int port) {
    this->_host = host;
    this->_port = port;
    _socket->connectToHost(host, port, QIODevice::ReadWrite);
}

void UDPClient::disconnectFromHost() {
    if (this->isConnected()) {
        _socket->disconnectFromHost();
        //timer->stop();
    }
}

void UDPClient::writeData(const char *buffer_tx, uint16_t length){
    if (_socket->isOpen()) {
        _socket->write(buffer_tx, length);
        _socket->flush();

#ifdef DEBUG_MSG_INTERFACES
            QByteArray txBuff = QByteArray::fromRawData(buffer_tx, length);
            qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
#endif
    }
}

void UDPClient::writeData(const QByteArray &txBuff){
    _socket->write(txBuff.data(), txBuff.length());
    _socket->flush();

#ifdef DEBUG_MSG_INTERFACES
        qCDebug(logDebug()) << QT_HEX << "UDP: tx - "  << qPrintable(txBuff.toHex(' '));
#endif
}

QByteArray UDPClient::readData(int timeoutMs) {
    QByteArray rxBuff = readDataWithTimeout(_socket, timeoutMs);

#ifdef DEBUG_MSG_INTERFACES
    qCDebug(logDebug()) << QT_HEX << "UDP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif

    return rxBuff;
}
