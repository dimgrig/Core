#include "UDPServer.h"

#include "Logger.h"

//QUdpServer::QUdpServer(QObject *parent) :
//     QObject(parent),
//    _udpSocket{ new QUdpSocket{this} },
//    _portNumber{6000},
//    _hostAddress{QHostAddress::Any},
//    _storedDatagrams{ std::queue<std::string>{} }
//{
//    connect(this->_udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

//    if(!this->_udpSocket->bind(this->_hostAddress, this->_portNumber)){
//        qCDebug(logDebug) << "UDPServer: not started";
//    } else {
//        qCDebug(logDebug) << "UDPServer: started";
//    }
//}

//QUdpServer::~QUdpServer()
//{
//    delete this->_udpSocket;
//}


///*Begin public members*/
//void QUdpServer::resetSocket(const QHostAddress &hostAddress, unsigned int portNumber)
//{
//    this->_hostAddress = hostAddress;
//    this->_portNumber = portNumber;
//    this->clearDatagrams();
//    this->_udpSocket->bind(this->_hostAddress, this->_portNumber);
//}

//bool QUdpServer::nextDatagramAvailable()
//{
//    return !(this->_storedDatagrams.empty());
//}

//std::string QUdpServer::nextDatagram()
//{
//    std::string returnString{this->_storedDatagrams.front()};
//    this->_storedDatagrams.pop();
//    return returnString;
//}
///*End public members*/

///*Begin private members*/


//void QUdpServer::clearDatagrams()
//{
//    while (!(this->_storedDatagrams.empty())) {
//        this->_storedDatagrams.pop();
//    }
//}

///*End private members*/

///*Begin private slots*/
//void QUdpServer::readPendingDatagrams()
//{
//    while (this->_udpSocket->hasPendingDatagrams()) {
//        QByteArray datagram;
//        datagram.resize(this->_udpSocket->pendingDatagramSize());
//        QHostAddress sender;
//        quint16 senderPort;
//        this->_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);\
//        processDatagram(datagram);
//    }
//    emit(datagramAvailable(this));
//}
///*End private slots*/



UDPServer::UDPServer(QObject *parent) :
    QObject(parent)
{
    _udp_socket = new QUdpSocket(this);

    connect(this->_udp_socket, &QUdpSocket::readyRead, this, &UDPServer::slotServerRead);

    if(!this->_udp_socket->bind(QHostAddress::Any, 8080)){
        qCDebug(logDebug) << "UDPServer: not started";
    } else {
        qCDebug(logDebug) << "UDPServer: started";
    }
}

void UDPServer::slotServerRead() {
    while (this->_udp_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(this->_udp_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 sender_port;
        this->_udp_socket->readDatagram(datagram.data(), datagram.size(), &sender, &sender_port);
        emit read_data(datagram, sender, sender_port);
    }
}

void UDPServer::writeData(const QByteArray &data, const QHostAddress sender, const quint16 sender_port) {
    QByteArray txBuff;
    QDataStream out(&txBuff, QIODevice::WriteOnly);
    out << qint64(0);
    out << data;
    out.device()->seek(qint64(0));
    out << qint64(data.size() - sizeof(qint64));
    this->_udp_socket->writeDatagram(data, sender, sender_port);
}
