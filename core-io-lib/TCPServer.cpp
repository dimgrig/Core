#include "TCPServer.h"

#include "Logger.h"

TCPServer::TCPServer(QObject *parent) :
    QObject(parent)
{
    _tcp_server = new QTcpServer(this);

    connect(_tcp_server, &QTcpServer::newConnection, this, &TCPServer::slotNewConnection);

    if(!_tcp_server->listen(QHostAddress::Any, 6000)){
        qCDebug(logDebug) << "TCPServer: not started";
    } else {
        qCDebug(logDebug) << "TCPServer: started";
    }
}

void TCPServer::slotNewConnection() {
    _tcp_socket = _tcp_server->nextPendingConnection();

    qCDebug(logDebug) << "TCPServer: new connection" << _tcp_socket->peerAddress();
    //_tcp_socket->write("Hello, World!!! I am echo server!\r\n");

    connect(_tcp_socket, &QTcpSocket::readyRead, this, &TCPServer::slotServerRead);
    connect(_tcp_socket, &QTcpSocket::disconnected, this, &TCPServer::slotClientDisconnected);
}

void TCPServer::slotServerRead() {
    while(_tcp_socket->bytesAvailable()>0) {
        QByteArray data = _tcp_socket->readAll();

        //_tcp_socket->write(array);
        emit read_data(data);
    }
}

void TCPServer::slotClientDisconnected() {
    _tcp_socket->close();

    qCDebug(logDebug) << "TCPServer: ";
}


void TCPServer::writeData(const QByteArray &data) {
    _tcp_socket->write(data);
}
