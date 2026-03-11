#include "MQTTClient.h"

#include "Logger.h"

MQTTClient::MQTTClient(QObject *parent) :
    QObject(parent)
{
    // auto connection_properties = this->_mqtt->connectionProperties();
    // connection_properties.setSessionExpiryInterval(UINT32_MAX);
    // this->_mqtt->setConnectionProperties(connection_properties);
    // qCDebug(logDebug()) << this->_mqtt->connectionProperties().maximumPacketSize();
    // qCDebug(logDebug()) << this->_mqtt->connectionProperties().maximumReceive();
    // qCDebug(logDebug()) << this->_mqtt->connectionProperties().maximumTopicAlias();

    //this->_mqtt->setProtocolVersion(QMqttClient::MQTT_3_1_1);
    this->_mqtt->setAutoKeepAlive(true);
    this->_mqtt->setKeepAlive(60);
    //qCDebug(logDebug()) << this->_mqtt->autoKeepAlive() << this->_mqtt->keepAlive();

    connect(this->timer, &QTimer::timeout, this, &MQTTClient::on_timer);
    connect(this->_mqtt, &QMqttClient::pingResponseReceived, [=](){
        //qCDebug(logDebug) << "mqtt ping" << this;
    });
    this->timer->start(1000);

    //connect(this->serial, &QSerialPort::errorOccurred, this, &SerialClient::handleError);
    connect(_mqtt, &QMqttClient::stateChanged, this, &MQTTClient::stateChanged);
    connect(_mqtt, &QMqttClient::disconnected, [=](){

    });
    connect(_mqtt, &QMqttClient::pingResponseReceived, [=]() {
        //QLatin1String(" PingResponse")
    });
    connect(_mqtt, &QMqttClient::messageReceived, this, &MQTTClient::onMessageReceived);
    connect(_mqtt, &QMqttClient::errorChanged, [=](QMqttClient::ClientError error){
        qCDebug(logInfo()) << "mqtt_client error" << error;

        if (error != QMqttClient::NoError) {
            // Можно попробовать исправить ошибку "на лету"
            if (error == QMqttClient::TransportInvalid) {
                // Сброс сокета и повторное подключение
                _mqtt->disconnectFromHost();
                QTimer::singleShot(100, this, [this]() {
                    qCDebug(logDebug()) << "Reconnecting after TransportInvalid...";
                    _mqtt->connectToHost();
                });
            } else {
                // Для других ошибок - стандартное поведение
                emit host_error(QString::number(error));
            }
        }
    });
}

MQTTClient::~MQTTClient() {
    disconnect(_mqtt, &QMqttClient::stateChanged, this, &MQTTClient::stateChanged);

    this->disconnectFromHost();
}

void MQTTClient::stateChanged() {
    switch (_mqtt->state()) {
    case QMqttClient::Connected:
        emit host_connected();
        qCDebug(logInfo()) << "MQTT:" << "connected" << _mqtt->hostname();
        //qCDebug(logInfo()) << "mqtt_client" << this;
        break;
    case QMqttClient::Disconnected:
        emit host_disconnected();
        qCDebug(logInfo()) << "MQTT:" << "disconnected";
        //qCDebug(logInfo()) << "mqtt_client" << this;
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

void MQTTClient::unsubscribe(QString topic) {
    _mqtt->unsubscribe(topic);
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

    if (_mqtt->state() == QMqttClient::Connected) {
        // Проверяем связь
        static int counter = 0;
        if (++counter % 30 == 0) { // Каждые 30 секунд
            ping();
        }
    } else if (_mqtt->state() == QMqttClient::Disconnected) {
        // Автопереподключение (опционально)
        //_mqtt->connectToHost();
    }
}
