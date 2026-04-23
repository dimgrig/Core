#ifndef MODBUSCLIENTSERVICE_H
#define MODBUSCLIENTSERVICE_H

#include "IModbusService.h"

#include <QObject>
#include <QMap>
#include <QMutex>

#include "ModbusClient.h" // существующий класс в ModbusCore
#include <QModbusDataUnit>

/*
 * ModbusClientService — обёртка над ModbusClient, предоставляющая чистый API.
 * - переводит сигналы ModbusClient::read_ready в registerValueRead
 * - предоставляет слоты для чтения/записи по имени регистра
 * - держит deviceUniqueId (строка, которую можно использовать в клиенте выше)
 */
class CORE_MODBUS_LIB_EXPORT ModbusClientService : public IModbusService {
    Q_OBJECT
public:
    // modbusClient может быть nullptr — тогда сервис будет работать в тестовом режиме до установки клиента
    explicit ModbusClientService(ModbusClient* modbusClient = nullptr, QObject* parent = nullptr);
    ~ModbusClientService() override;

    // IModbusService
public slots:
    void connectToHost() override;
    void disconnectFromHost() override;

    void readRegister(int serverAddress, const QString& registerName) override;
    void writeRegister(int serverAddress, const QString& registerName, const QVariant& value) override;

    void setDeviceUniqueId(const QString& uniqueId) override;
    QString deviceUniqueId() const override;

    // Дополнительные утилиты
    void setModbusClient(ModbusClient* client);

private slots:
    // slot для приема QModbusDataUnit от ModbusClient
    void onReadReady(int serverAddress, const QModbusDataUnit& mdu);
    void onHostConnected();
    void onHostDisconnected();
    void onHostError(const QString& error);

private:
    // найдет mdu mapping по registerName
    bool findMduByName(int serverAddress, const QString& name, QModbusDataUnit& outMdu) const;

    // пробует сопоставить incoming mdu с записью в mdum и эмитить значение
    void handleIncomingMdu(int serverAddress, const QModbusDataUnit& incoming);

    ModbusClient* _modbusClient = nullptr;
    QString _deviceUniqueId;
    mutable QMutex _mutex;
};

#endif // MODBUSCLIENTSERVICE_H
