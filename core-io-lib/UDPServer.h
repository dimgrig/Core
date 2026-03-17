#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include <QHostAddress>

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT UDPServer : public QObject {
Q_OBJECT
public:
    UDPServer(QObject *parent = nullptr);
signals:
    void read_data(QByteArray rxBuff, QHostAddress sender, quint16 senderPort);
public slots:
    void slotServerRead();
    void writeData(const QByteArray &data, const QHostAddress sender, const quint16 sender_port);
private:
    QUdpSocket * _udp_socket;
};

#endif // UDPSERVER_H
