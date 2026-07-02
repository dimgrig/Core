#include "MockMQTTClient.h"

MockMQTTClient::MockMQTTClient(QObject *parent)
    : MQTTClient(parent)
{
    // Не запускаем таймеры, не подключаемся к реальному брокеру
    setConnected(true);   // по умолчанию "подключены"
}

void MockMQTTClient::connectToHost(const QString &host, int port) {
    Q_UNUSED(host);
    Q_UNUSED(port);
    setConnected(true);
    emit host_connected();
}

void MockMQTTClient::disconnectFromHost() {
    setConnected(false);
    emit host_disconnected();
}

void MockMQTTClient::subscribe(const QString &topic) {
    m_subscribed.insert(topic);
}

void MockMQTTClient::unsubscribe(const QString &topic) {
    m_subscribed.remove(topic);
}

// Перегрузки publish
void MockMQTTClient::publish(const QString &topic, const QString &message) {
    m_published[topic] = message;
    // По желанию можно сразу эмулировать ответное сообщение
    // например, если topic – это запрос оператору, автоматически отвечаем
}

void MockMQTTClient::publish(const QString &topic, const QByteArray &message) {
    publish(topic, QString::fromUtf8(message));
}

void MockMQTTClient::publish(const QString &topic, std::string message) {
    publish(topic, QString::fromStdString(message));
}

// Эмуляция входящего сообщения
void MockMQTTClient::simulateMessage(const QString &topic, const QString &message) {
    emit messageReceived(topic, message);
}

// Доступ к последним опубликованным сообщениям
QString MockMQTTClient::lastPublished(const QString &topic) const {
    return m_published.value(topic);
}
