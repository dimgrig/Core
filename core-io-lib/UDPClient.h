#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QUdpSocket>
#include <QTimer>
#include <QWidget>

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT UDPClient : public QWidget {
Q_OBJECT
public:
    UDPClient(QWidget *parent = nullptr);
    virtual ~UDPClient();

    void set_async(bool async);
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
    //void open(int port);
    //void close();
    void writeData(const char *buffer_tx, uint16_t length);
    void writeData(const QByteArray &txBuff);
    std::string readData();
private:
    //void readData();
#if QT_VERSION > QT_VERSION_CHECK(5, 13, 0)
    void handleError(QAbstractSocket::SocketError error);
#endif
    void readDataSlot();
//    void on_timer();
private:
    void setSockOpt();

    bool _async = false;

    QUdpSocket *_socket = new QUdpSocket(this);

    int m_port = 0;
    int m_tx_port = 0;
};

#endif // UDPCLIENT_H
