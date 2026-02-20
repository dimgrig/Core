#ifndef IMODBUSSERVICE_H
#define IMODBUSSERVICE_H

#include <QObject>
#include <QString>
#include <QVariantList>

#include "core-modbus-lib-global.h"

/*
 * IModbusService — высокоуровневый интерфейс сервиса Modbus.
 * Подписывайтесь на сигналы, вызывайте слоты для чтения/записи.
 */
class CORE_MODBUS_LIB_EXPORT IModbusService : public QObject {
    Q_OBJECT
public:
    explicit IModbusService(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IModbusService() = default;

public slots:
    virtual void connectToHost() = 0;
    virtual void disconnectFromHost() = 0;

    // Выполнить чтение регистра (registerName — имя регистра, как задано в mapping)
    virtual void readRegister(const QString& registerName) = 0;

    // Записать значение в регистр (value — тип зависит от регистра)
    virtual void writeRegister(const QString& registerName, const QVariant& value) = 0;

    // Немного управления
    virtual void setDeviceUniqueId(const QString& uniqueId) = 0;
    virtual QString deviceUniqueId() const = 0;

signals:
    // Результат чтения: имя регистра и список 16-битных значений (QVariantList<int>)
    void registerValueRead(const QString& registerName, const QVariantList& values);

    // Подключение/отключение устройства
    void deviceConnected(const QString& uniqueId);
    void deviceDisconnected(const QString& uniqueId);

    // Сообщения/ошибки
    void errorOccurred(const QString& error);
};

#endif // IMODBUSSERVICE_H
