#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT TCPServer : public QObject
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = nullptr);
signals:
    void read_data(QByteArray rxBuff);
public slots:
    void slotNewConnection();
    void slotServerRead();
    void slotClientDisconnected();
    void writeData(const QByteArray &data);
private:
    QTcpServer * _tcp_server;
    QTcpSocket * _tcp_socket;
};

#endif // TCPSERVER_H
