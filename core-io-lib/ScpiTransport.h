#ifndef SCBITRANSPORT_H
#define SCBITRANSPORT_H

#include <QObject>
#include <QByteArray>
#include <QString>

#include <memory>

#include <QEventLoop>
#include <QSerialPort>
#include <QTcpSocket>
#include <QTimer>

class ScpiTransport : public QObject
{
    Q_OBJECT
public:
    explicit ScpiTransport(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ScpiTransport() {}

    // Методы управления соединением
    virtual void connectToDevice() = 0;
    virtual void disconnectFromDevice() = 0;
    virtual bool isConnected() const = 0;

    // Отправка команды и получение ответа
    virtual void sendCommand(const QByteArray &command) = 0;
    virtual QByteArray sendQuery(const QByteArray &query) = 0;

    // Установка таймаута (если нужен для транспорта)
    virtual void setTimeout(int ms) = 0;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &errorString);
};

class TcpTransport : public ScpiTransport
{
    Q_OBJECT
public:
    explicit TcpTransport(const QString &host, quint16 port, QObject *parent = nullptr);
    ~TcpTransport();

    void connectToDevice() override;
    void disconnectFromDevice() override;
    bool isConnected() const override;
    void sendCommand(const QByteArray &command) override;
    QByteArray sendQuery(const QByteArray &query) override;
    void setTimeout(int ms) override { m_timeoutMs = ms; }

private slots:
    void onConnected();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *m_socket;
    QTimer *m_timeoutTimer;
    QEventLoop *m_eventLoop;
    QString m_host;
    quint16 m_port;
    int m_timeoutMs;
};

class SerialTransport : public ScpiTransport
{
    Q_OBJECT
public:
    explicit SerialTransport(const QString &portName,
                             qint32 baudRate = QSerialPort::Baud9600,
                             QSerialPort::DataBits dataBits = QSerialPort::Data8,
                             QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                             QSerialPort::Parity parity = QSerialPort::NoParity,
                             QObject *parent = nullptr);
    ~SerialTransport();

    void connectToDevice() override;
    void disconnectFromDevice() override;
    bool isConnected() const override;
    void sendCommand(const QByteArray &command) override;
    QByteArray sendQuery(const QByteArray &query) override;
    void setTimeout(int ms) override { m_timeoutMs = ms; }

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serial;
    QTimer *m_timeoutTimer;
    QEventLoop *m_eventLoop;
    QEventLoop *m_currentEventLoop; // текущий активный event loop (для onReadyRead)
    QString m_portName;
    qint32 m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::Parity m_parity;
    int m_timeoutMs;

    QByteArray m_readBuffer;
};

#endif // SCBITRANSPORT_H
