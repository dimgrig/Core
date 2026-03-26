#ifndef SCPIDEVICE_H
#define SCPIDEVICE_H

#include <QObject>
#include <QMessageBox>
#include <QString>
#include <QTcpSocket>
#include <QHostAddress>
#include <QEventLoop>
#include <QTimer>

#include <memory>
#include <cassert>
#include <exception>

#include "mixins.h"

#include "ScpiTransport.h"

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT ScpiDevice : public QObject, NoCopyNoMoveBase {
Q_OBJECT
public:
    // A general device error (unable to connect, invalid IP, etc.)
    struct ScpiDeviceError : public std::exception {
        explicit ScpiDeviceError(const QString &errstr) : errorDescription(errstr.toUtf8()) {}
        const char * what() const throw() override {
            return errorDescription.constData();}
        private: QByteArray errorDescription;
    };

    // An error perfirming a query (timeout, unexpected OPC response, etc.)
    struct ScpiQueryError : public ScpiDeviceError {
        explicit ScpiQueryError(const QString &errstr) : ScpiDeviceError(errstr) {}
    };

    void connectToDevice();
    void disconnectFromDevice() noexcept;
    bool is_connected() noexcept;
    void reset();       // Reset device
    void clearState();  // Clear device's state register
    QString getIDN();   // Request identification string
    void setTimeout(unsigned int scpi_timeout_ms); // Set SCPI connection timeout
signals:
    void connected();
    void connecting();
    void disconnected();
    void needReconnect();
    void reconnectingStarted();
    void reconnectingFinished();
public slots:
    void connectRequest();
    void disconnectRequest();
    void tryReconnect();
private slots:
    void onConnected();
    void onDisconnected();
    void onTransportError(const QString &errorString);
    //void onSocketError(QAbstractSocket::SocketError socket_error);
    //void recreateSocket();
    void startReconnect();
protected:
    // Для Ethernet
    ScpiDevice(const QString &ip, unsigned int port, QObject *parent = nullptr);
    // Для Serial
    ScpiDevice(const QString &portName,
               qint32 baudRate = QSerialPort::Baud115200,
               QSerialPort::DataBits dataBits = QSerialPort::Data8,
               QSerialPort::StopBits stopBits = QSerialPort::OneStop,
               QSerialPort::Parity parity = QSerialPort::NoParity,
               QObject *parent = nullptr);
    virtual ~ScpiDevice();

//    void sendCommand(const QString &command);   // Sends a command with completion request
//    QString sendQuery(const QString &query);    // Sends a query and recieves a response for it
    void sendCommand(const QString &command);   // Sends a command
    QByteArray sendQuery(const QString &query);    // Send a query and recieves a response for it

private:
    void initTransport();

    std::unique_ptr<ScpiTransport> m_transport;

    QTimer* _reconnect_timer = nullptr;
    bool _is_reconnecting = false;

    unsigned int _timeout_ms = 5000;

    // QHostAddress _ip = QHostAddress("0.0.0.0");
    // unsigned int _port = 0;
    // unsigned int _timeout_ms = 5000;
    // unsigned int _event_timeout_ms = 1100;

    // QString _comPort;

    // std::function<void(QObject*)> _qobj_deleter = [](QObject* obj) {obj->deleteLater();};
    // std::unique_ptr<QTcpSocket,decltype(_qobj_deleter)> _socket = nullptr;

    // // Are used to imitate waitForXXX behaviour
    // std::unique_ptr<QTimer,decltype(_qobj_deleter)> _timeout_timer = nullptr;
    // std::unique_ptr<QEventLoop,decltype(_qobj_deleter)> _local_event_loop = nullptr;
};

template<typename F>
void safe_device_call(QWidget *parent, F &&f) {
    try {
        f();
    } catch (const ScpiDevice::ScpiDeviceError &err) {
        QMessageBox::warning(parent, "Ошибка связи с устройством", err.what());
    } catch (const std::exception &err) {
        QMessageBox::warning(parent, "Неизвестная ошибка", err.what());
    }
}

#define SAFE_CALL_DEVICE(parent, expr) \
safe_device_call(parent, [&]{ expr; })

#endif // SCPIDEVICE_H
