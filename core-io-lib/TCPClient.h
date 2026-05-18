#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QAbstractSocket>
#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWidget>

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT TCPClient : public QWidget {
Q_OBJECT
public:
    TCPClient(QWidget *parent = nullptr);
    virtual ~TCPClient();

    bool isConnected() {return this->_socket->isOpen();};
    bool isReadable() {return this->_socket->isReadable();};
    int bytesAvailable() {return this->_socket->bytesAvailable();};
    bool waitForReadyRead(int msecs = 1000) {return this->_socket->waitForReadyRead(msecs);};
    bool reset() const {return this->_socket->reset();};
signals:
    void host_error(QString error);
    void host_connected();
    void host_disconnected();
    void read_data(QByteArray rxBuff);
public slots:
    void connectToHost(QString host, int port);
    void disconnectFromHost();
    void writeData(const char *buffer_tx, uint16_t length);
    void writeData(const QByteArray &txBuff);
    QByteArray readData(int timeoutMs = 3000);
private slots:
#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
    void handleError(QAbstractSocket::SocketError error);
#endif
private:
    void setSockOpt();

    QString _host;
    int _port = 0;

    QTcpSocket* _socket = new QTcpSocket(this);
};

#endif // TCPCLIENT_H
