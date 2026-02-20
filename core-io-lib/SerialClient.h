#ifndef SERIALCLIENT_H
#define SERIALCLIENT_H

#include <QSerialPort>
#include <QTimer>
#include <QWidget>

#include "core-io-lib-global.h"

struct CORE_IO_LIB_EXPORT SerialSettings {
    QString name;
    qint32 baudRate;
    QString stringBaudRate;
    QSerialPort::DataBits dataBits;
    QString stringDataBits;
    QSerialPort::Parity parity;
    QString stringParity;
    QSerialPort::StopBits stopBits;
    QString stringStopBits;
    QSerialPort::FlowControl flowControl;
    QString stringFlowControl;
    bool localEchoEnabled;
};

class CORE_IO_LIB_EXPORT SerialClient : public QObject {
Q_OBJECT
public:
    SerialClient(QObject *parent = nullptr);
    virtual ~SerialClient();

    void set_async(bool async);
    bool isConnected() {return this->_serial->isOpen();};
    bool isReadable() {return this->_serial->isReadable();};
    int bytesAvailable() {return this->_serial->bytesAvailable();};
    bool waitForReadyRead(int msecs = 1000) {return this->_serial->waitForReadyRead(msecs);};
    bool reset() {return false;};
signals:
    void port_error(QString error);
    void port_opened();
    void port_closed();
    void read_data(QByteArray rxBuff);
public slots:
    void openSerialPort(QString port, int speed);
    void closeSerialPort();
    void writeData(const char *buffer_tx, uint16_t length);
    void writeData(const QByteArray &txBuff);
    std::string readData();
private slots:
    void handleError(QSerialPort::SerialPortError error);
    void readDataSlot();
    void on_timer();
private:
    bool _async = false;

    QSerialPort *_serial = new QSerialPort(this);

    QTimer *timer = new QTimer(this);
    int TIMER_MS = 1000;

    QByteArray _rxBuff_overflow;
};
#endif // SERIALCLIENT_H
