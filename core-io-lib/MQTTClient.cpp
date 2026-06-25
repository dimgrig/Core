#include "MQTTClient.h"

// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include <sys/socket.h>

#include "Logger.h"

#include "Categories.h"
#include "JournalAccess.h"

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
    this->_mqtt->setAutoKeepAlive(false);
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
    connect(_mqtt, &QMqttClient::messageReceived, this, &MQTTClient::onMessageReceived);
    connect(_mqtt, &QMqttClient::errorChanged, [=](QMqttClient::ClientError error){
        qCDebug(logInfo()) << "mqtt_client error" << error;

        // if (error != QMqttClient::NoError) {
        //     // Можно попробовать исправить ошибку "на лету"
        //     if (error == QMqttClient::TransportInvalid) {
        //         // Сброс сокета и повторное подключение
        //         _mqtt->disconnectFromHost();
        //         QTimer::singleShot(100, this, [this]() {
        //             qCDebug(logDebug()) << "Reconnecting after TransportInvalid...";
        //             _mqtt->connectToHost();
        //         });
        //     } else {
        //         // Для других ошибок - стандартное поведение
        //         emit host_error(QString::number(error));
        //     }
        // }

        if (error == QMqttClient::TransportInvalid) {
            if (!_reconnecting) {
                _reconnecting = true;
                // Не вызываем disconnectFromHost() — клиент уже отключён
                QTimer::singleShot(2000, this, [this]() {
                    _reconnecting = false;
                    if (_mqtt->state() == QMqttClient::Disconnected) {
                        qCDebug(logDebug()) << "Reconnecting after delay...";
                        // Используем ранее сохранённые хост/порт или задаём явно
                        _mqtt->connectToHost(); // если setHostname/setPort уже были вызваны
                    }
                });
            }
        } else if (error != QMqttClient::NoError) {
            emit host_error(QString::number(error));
        }
    });
}

MQTTClient::~MQTTClient() {
    this->timer->stop();

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
            if (auto* j = JournalAccess::journal())
                j->warning(Category::IO, QString("MQTT: ошибка %1").arg(QString::number(_mqtt->error())));
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
        if (auto* j = JournalAccess::journal())
            j->warning(Category::IO, QString("MQTT: не удалось подписаться на топик %1").arg(topic));
    }
}

void MQTTClient::unsubscribe(QString topic) {
    _mqtt->unsubscribe(topic);
}

void MQTTClient::publish(QString topic, QByteArray message) {
    if (_mqtt->publish(topic, message) == -1) {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::IO, QString("MQTT: не удалось опубликовать в топик %1").arg(topic));
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
        if (++_timer_counter % 30 == 0) { // Каждые 30 секунд
            //not working - leads mqtt_client error QMqttClient::TransportInvalid every 90 secs
            ping();

            // // Вместо ping() отправляем keep-alive сообщение
            // QByteArray payload = QByteArray::number(_timer_counter);
            // // Используем системный топик, начинающийся с '$'
            // if (_mqtt->publish(QStringLiteral("__mqtt_keepalive/"), payload, 1) == -1) {
            //     qCWarning(logDebug()) << "Failed to send keep-alive publish";
            // } else {
            //     qCDebug(logDebug()) << "Keep-alive sent:" << payload;
            // }
        }
    } else if (_mqtt->state() == QMqttClient::Disconnected) {
        // Автопереподключение (опционально)
        //_mqtt->connectToHost();
    }
}

// MQTTClient::MQTTClient(QObject *parent)
//     : QObject(parent)
// {
//     mosquitto_lib_init();
//     _timer.setInterval(100); // 10 раз в секунду – достаточно
//     connect(&_timer, &QTimer::timeout, this, &MQTTClient::onTimer);
// }

// MQTTClient::~MQTTClient()
// {
//     disconnectFromHost();
//     mosquitto_lib_cleanup();
// }

// void MQTTClient::connectToHost(const QString &host, int port)
// {
//     if (_mosq) {
//         mosquitto_destroy(_mosq);
//         _mosq = nullptr;
//     }

//     _host = host;
//     _port = port;

//     // Создаём новый экземпляр. ID можно сгенерировать или оставить nullptr для автоматического
//     _mosq = mosquitto_new(nullptr, true, this);
//     if (!_mosq) {
//         emit host_error("Failed to create mosquitto instance");
//         return;
//     }

//     // Регистрируем коллбэки
//     mosquitto_connect_callback_set(_mosq, onConnectCallback);
//     mosquitto_disconnect_callback_set(_mosq, onDisconnectCallback);
//     mosquitto_message_callback_set(_mosq, onMessageCallback);
//     mosquitto_log_callback_set(_mosq, onLogCallback);
//     mosquitto_publish_callback_set(_mosq, onPublishCallback);

//     // Устанавливаем keep-alive (стандартный механизм, который работает)
//     mosquitto_int_option(_mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V311);
//     //mosquitto_int_option(_mosq, MOSQ_OPT_KEEPALIVE, _keepAlive);

//     // Подключаемся
//     int rc = mosquitto_connect(_mosq, host.toUtf8().constData(), port, _keepAlive);
//     if (rc != MOSQ_ERR_SUCCESS) {
//         emit host_error(QString("Connect failed: %1").arg(mosquitto_strerror(rc)));
//         mosquitto_destroy(_mosq);
//         _mosq = nullptr;
//         return;
//     }

//     _timer.start();
//     //qCDebug(logDebug()) << "Mosquitto connecting to" << host << port;
// }

// void MQTTClient::disconnectFromHost()
// {
//     _timer.stop();
//     if (_mosq) {
//         mosquitto_disconnect(_mosq);
//         // даём немного времени на отправку DISCONNECT перед уничтожением
//         mosquitto_loop(_mosq, 100, 1);
//         mosquitto_destroy(_mosq);
//         _mosq = nullptr;
//     }
//     _connected = false;
// }

// void MQTTClient::subscribe(const QString &topic, int qos)
// {
//     if (_mosq) {
//         int rc = mosquitto_subscribe(_mosq, nullptr, topic.toUtf8().constData(), qos);
//         if (rc != MOSQ_ERR_SUCCESS)
//             qCWarning(logDebug()) << "Subscribe failed:" << mosquitto_strerror(rc);
//     }
// }

// void MQTTClient::unsubscribe(const QString &topic)
// {
//     if (_mosq)
//         mosquitto_unsubscribe(_mosq, nullptr, topic.toUtf8().constData());
// }

// void MQTTClient::publish(const QString &topic, const QByteArray &message, int qos, bool retain)
// {
//     if (_mosq) {
//         int rc = mosquitto_publish(_mosq, nullptr, topic.toUtf8().constData(),
//                                    message.size(), message.constData(), qos, retain);
//         if (rc != MOSQ_ERR_SUCCESS)
//             qCWarning(logDebug()) << "Publish failed:" << mosquitto_strerror(rc);
//     }
// }

// void MQTTClient::publish(const QString &topic, const QString &message, int qos, bool retain)
// {
//     publish(topic, message.toUtf8(), qos, retain);
// }

// void MQTTClient::publish(const QString &topic, const std::string &message, int qos, bool retain)
// {
//     publish(topic, QString(message.c_str()), qos, retain);
// }

// void MQTTClient::onTimer()
// {
//     if (_mosq) {
//         int rc = mosquitto_loop(_mosq, 0, 1);
//         if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_NO_CONN) {
//             // ошибка, вероятно разрыв
//             qCDebug(logDebug()) << "mosquitto_loop error:" << mosquitto_strerror(rc);
//             if (_connected) {
//                 _connected = false;
//                 emit host_disconnected();
//             }
//             _timer.stop();
//         }
//     }
// }

// // Статические коллбэки
// void MQTTClient::onConnectCallback(struct mosquitto *, void *obj, int rc)
// {
//     auto *self = static_cast<MQTTClient*>(obj);
//     if (rc == MOSQ_ERR_SUCCESS) {
//         self->_connected = true;
//         emit self->host_connected();
//         qCDebug(logDebug()) << "MQTTClient connected to" << self->_host;
//     } else {
//         self->_connected = false;
//         emit self->host_error(QString("Connection refused: %1").arg(mosquitto_strerror(rc)));
//     }
// }

// void MQTTClient::onDisconnectCallback(struct mosquitto *, void *obj, int rc)
// {
//     auto *self = static_cast<MQTTClient*>(obj);
//     if (self->_connected) {
//         self->_connected = false;
//         emit self->host_disconnected();
//         qCDebug(logDebug()) << "MQTTClient disconnected, rc=" << rc;
//     }
//     self->_timer.stop();
// }

// void MQTTClient::onMessageCallback(struct mosquitto *, void *obj, const struct mosquitto_message *msg)
// {
//     auto *self = static_cast<MQTTClient*>(obj);
//     QString topic = QString::fromUtf8(msg->topic);
//     QString payload = QString::fromUtf8(static_cast<const char*>(msg->payload), msg->payloadlen);
//     emit self->messageReceived(topic, payload);
// }

// void MQTTClient::onLogCallback(struct mosquitto *, void *obj, int level, const char *str)
// {
//     Q_UNUSED(obj);
//     // Перенаправляем в ваш логгер
//     //qCDebug(logDebug()) << "mosquitto:" << str;
// }

// void MQTTClient::onPublishCallback(struct mosquitto *, void *obj, int mid)
// {
//     Q_UNUSED(obj);
//     //qCDebug(logDebug()) << "Published message id:" << mid;
// }
