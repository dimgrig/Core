#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QMqttClient>
#include <QTimer>
#include <QWidget>

#include "core-io-lib-global.h"

class CORE_IO_LIB_EXPORT MQTTClient : public QObject {
Q_OBJECT
public:
    MQTTClient(QObject *parent = nullptr);
    virtual ~MQTTClient();

    bool isConnected() {return (this->_mqtt->state() == QMqttClient::ClientState::Connected);};
    bool reset() {return false;};
signals:
    void host_error(QString error);
    void host_connected();
    void host_disconnected();
    //void messageReceived(QString topic, QByteArray message);
    void messageReceived(QString topic, QString message);
public slots:
    void stateChanged();
    void connectToHost(QString host = "localhost", int port = 1883);
    void disconnectFromHost();
    void subscribe(QString topic);
    void unsubscribe(QString topic);
    void publish(QString topic, QByteArray message);
    void publish(QString topic, QString message);
    void publish(QString topic, std::string message);
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
    void ping() {this->_mqtt->requestPing();};
private slots:
    //void handleError(QSerialPort::SerialPortError error);
    void on_timer();
private:
    QMqttClient *_mqtt = new QMqttClient(this);

    QTimer *timer = new QTimer(this);
    int TIMER_MS = 1000;
};
#endif // MQTTCLIENT_H
