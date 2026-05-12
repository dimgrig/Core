#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <QString>

#include <QMqttClient>

// #include <mosquitto.h>

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

    bool _reconnecting = false;

    long long _timer_counter = 0;
};



// class CORE_IO_LIB_EXPORT MQTTClient : public QObject
// {
//     Q_OBJECT
// public:
//     explicit MQTTClient(QObject *parent = nullptr);
//     virtual ~MQTTClient();

//     bool isConnected() const { return _connected; }
//     bool reset() {return false;};
// signals:
//     void host_error(const QString &error);
//     void host_connected();
//     void host_disconnected();
//     void messageReceived(const QString &topic, const QString &message);

// public slots:
//     void connectToHost(const QString &host = "localhost", int port = 1883);
//     void disconnectFromHost();
//     void subscribe(const QString &topic, int qos = 0);
//     void unsubscribe(const QString &topic);
//     void publish(const QString &topic, const QByteArray &message, int qos = 0, bool retain = false);
//     void publish(const QString &topic, const QString &message, int qos = 0, bool retain = false);
//     void publish(const QString &topic, const std::string &message, int qos = 0, bool retain = false);

// private slots:
//     void onTimer(); // вызывает mosquitto_loop()

// private:
//     static void onConnectCallback(struct mosquitto *mosq, void *obj, int rc);
//     static void onDisconnectCallback(struct mosquitto *mosq, void *obj, int rc);
//     static void onMessageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
//     static void onLogCallback(struct mosquitto *mosq, void *obj, int level, const char *str);
//     static void onPublishCallback(struct mosquitto *mosq, void *obj, int mid);

//     struct mosquitto *_mosq = nullptr;
//     QTimer _timer;
//     bool _connected = false;
//     QString _host;
//     int _port = 1883;
//     int _keepAlive = 60; // можно подстроить
// };

#endif // MQTTCLIENT_H
