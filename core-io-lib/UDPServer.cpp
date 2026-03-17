#include "UDPServer.h"

#include "Logger.h"

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
