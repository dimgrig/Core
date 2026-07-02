#ifndef MOCKMQTTCLIENT_H
#define MOCKMQTTCLIENT_H

#include <QSet>
#include <QMap>

#include "MQTTClient.h"

class MockMQTTClient : public MQTTClient {
    Q_OBJECT
public:
    explicit MockMQTTClient(QObject *parent = nullptr);

    void connectToHost(const QString &host = "localhost", int port = 1883);
    void disconnectFromHost();
    void subscribe(const QString &topic);
    void unsubscribe(const QString &topic);

    // Перегрузки publish
    void publish(const QString &topic, const QString &message);
    void publish(const QString &topic, const QByteArray &message);
    void publish(const QString &topic, std::string message);

    // Эмуляция входящего сообщения
    void simulateMessage(const QString &topic, const QString &message);

    // Доступ к последним опубликованным сообщениям
    QString lastPublished(const QString &topic) const;

    QMap<QString, QString> allPublished() const { return m_published; }

    // Управление состоянием подключения
    void setConnected(bool c) { m_connected = c; }
    bool isConnected() const { return m_connected; }

    // Для проверки подписок
    bool isSubscribed(const QString &topic) const {
        return m_subscribed.contains(topic);
    }

private:
    bool m_connected = false;
    QSet<QString> m_subscribed;
    QMap<QString, QString> m_published;
};

#endif // MOCKMQTTCLIENT_H
