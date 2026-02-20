#include "MQTTClient.h"

#include "Logger.h"

MQTTClient::MQTTClient(QObject *parent) :
    QObject(parent)
{
    //qCDebug(logDebug()) << ;
    //connect(this->serial, &QSerialPort::errorOccurred, this, &SerialClient::handleError);
    connect(_mqtt, &QMqttClient::stateChanged, this, &MQTTClient::stateChanged);
    connect(_mqtt, &QMqttClient::disconnected, [=](){

    });
    connect(_mqtt, &QMqttClient::pingResponseReceived, [=]() {
        //QLatin1String(" PingResponse")
    });
    connect(_mqtt, &QMqttClient::messageReceived, this, &MQTTClient::onMessageReceived);
}

MQTTClient::~MQTTClient() {
    this->disconnectFromHost();
}

void MQTTClient::stateChanged() {
    switch (_mqtt->state()) {
        case QMqttClient::Connected:
            emit host_connected();
            qCDebug(logInfo()) << "MQTT:" << "connected" << _mqtt->hostname();
        break;
        case QMqttClient::Disconnected:
            emit host_disconnected();
            qCDebug(logInfo()) << "MQTT:" << "disconnected";
        break;
        case QMqttClient::Connecting:
    //                qCDebug(logInfo()) << "MQTT: " << "connecting"
    //                                   << _mqtt->hostname()
    //                                   << _mqtt->port()
    //                                   << _mqtt->password()
    //                                   << _mqtt->clientId();
        break;
    }
}

void MQTTClient::connectToHost(QString host, int port) {
    _mqtt->setHostname(host);
    _mqtt->setPort(port);
    _mqtt->connectToHost();
    if (this->isConnected()) {
        //timer->start(this->TIMER_MS);
    } else {
        if (_mqtt->error() != QMqttClient::ClientError::NoError) {
            emit host_error("MQTT: Error" + QString::number(_mqtt->error()));
            volatile QString str = QString::number(_mqtt->error());
            qCDebug(logCritical()) << "MQTT: Error" << QString::number(_mqtt->error());
        }
    }
}

void MQTTClient::disconnectFromHost() {
    if (_mqtt && this->isConnected()) {
        _mqtt->disconnectFromHost();
        //timer->stop();
    }
}

void MQTTClient::subscribe(QString topic) {
    auto subscription = _mqtt->subscribe(topic);
    if (!subscription) {
        qCDebug(logCritical()) << "MQTT: Could not subscribe";
    }
}

void MQTTClient::publish(QString topic, QByteArray message) {
    if (_mqtt->publish(topic, message) == -1) {
        qCDebug(logCritical()) << "MQTT: Could not publish message";
    }
}

void MQTTClient::publish(QString topic, QString message) {
    this->publish(topic, message.toUtf8());
}

void MQTTClient::publish(QString topic, std::string message) {
    this->publish(topic, QString(message.c_str()));
}

void MQTTClient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic) {
    //emit messageReceived(topic.name(), message);
    emit messageReceived(topic.name(), QString::fromUtf8(message));

#ifdef DEBUG_MSG_INTERFACES
        qCDebug(logDebug()) << QT_HEX << "MQTT: rx -" << topic.name() << "-" << message;
#endif
}

void MQTTClient::on_timer() {
    //TODO BASE check connection???
}
