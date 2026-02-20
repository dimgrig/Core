#include "UDPClient.h"

#include "Logger.h"

UDPClient::UDPClient(QWidget *parent) :
        QWidget(parent)
{

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

    switch(socketError){
        case QAbstractSocket::RemoteHostClosedError:
            qCDebug(logWarning()) << "UDP: RemoteHostClosedError";
        break;
        case QAbstractSocket::HostNotFoundError:
            qCDebug(logWarning()) << "UDP: HostNotFoundError. Connection was refused. Server is not working";
        break;
        case QAbstractSocket::ConnectionRefusedError:
            qCDebug(logWarning()) << "UDP: ConnectionRefusedError. Client. The host was not found";
        break;
        default:
            qCDebug(logWarning()) << "UDP: Error" << _socket->errorString();
        break;
    }
}
#endif

void UDPClient::connectToHost(QString host, int port) {
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

std::string UDPClient::readData() {
    QByteArray rxBuff;
    while(_socket->bytesAvailable()){
        rxBuff += _socket->readAll();

#ifdef DEBUG_MSG_INTERFACES
            qCDebug(logDebug()) << QT_HEX << "UDP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif
    }

    return rxBuff.toStdString();
}

void UDPClient::readDataSlot(){
    QByteArray rxBuff;
    while(_socket->bytesAvailable()){
        rxBuff += _socket->readAll();

#ifdef DEBUG_MSG_INTERFACES
            qCDebug(logDebug()) << QT_HEX << "UDP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif
    }

    emit read_data(std::move(rxBuff));
}

//void UDPClient::open(int port) {
//    m_port = port;
//    if ((m_port % 2) == 0) {
//        m_tx_port = m_port + 1;
//    } else {
//        m_tx_port = m_port - 1;
//    }
//    if (udp_socket->bind(QHostAddress::Any, m_port)) {
//        qCDebug(logDebug()) << "Success binding port " << m_port;
//        connect(udp_socket, &QUdpSocket::readyRead, this, &UDPClient::readData);
//        //udp_send(QString("me"), WHO_IS_ONLINE);
////    } else if (udp_socket->bind(QHostAddress::Any, m_port + 1)) {
////        qCDebug(logDebug()) << "Success binding port " << m_port + 1;
////        connect(udp_socket, &QUdpSocket::readyRead, this, &UDPClient::readData);
////        //udp_send(QString("me"), WHO_IS_ONLINE);
//    } else {
//        qCDebug(logDebug()) << "Error binding port " << m_port;
//    }
//}

//void UDPClient::close()
//{
//    if (this->isConnected()) {
//        udp_socket->close();
//        //timer->stop();
//        qCDebug(logInfo()) << "UDP: " << "disconnected";
//    }
//}

//void UDPClient::writeData(const QByteArray &txBuff) {
//    QByteArray data;
//    QDataStream out(&data, QIODevice::WriteOnly);
//    out << qint64(0);
//    out << txBuff;
//    out.device()->seek(qint64(0));
//    out << qint64(data.size() - sizeof(qint64));
//    _socket->writeDatagram(data, QHostAddress::Broadcast, m_tx_port);
//    if (cfg.DEBUG_MSG_ETHERNET) {
//        qCDebug(logDebug()) << QT_HEX << "UDP: tx - "  << qPrintable(txBuff.toHex(' '));
//    }
//}

//void UDPClient::readData() {
//    QByteArray datagram;
//    datagram.resize(_socket->pendingDatagramSize());
//    QHostAddress *address = new QHostAddress();
//    _socket->readDatagram(datagram.data(), datagram.size(), address);
//    delete address;

//    QDataStream in(&datagram, QIODevice::ReadOnly);

//    qint64 size = -1;
//    if(in.device()->size() > (qint64) sizeof(qint64)) {
//        in >> size;
//    }
//    else {
//        return;
//    }

//    if (in.device()->size() - (qint64) sizeof(qint64) < size) {
//        return;
//    }

//    QByteArray rxBuff;
//    in >> rxBuff;

//    if (cfg.DEBUG_MSG_ETHERNET) {
//        qCDebug(logDebug()) << QT_HEX << "UDP: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
//    }
//    emit read_data(rxBuff);
//}
