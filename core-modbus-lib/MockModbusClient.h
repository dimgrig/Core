#ifndef MOCKMODBUSCLIENT_H
#define MOCKMODBUSCLIENT_H

#include <map>
#include <QVector>

#include "ModbusClient.h"

#include "core-modbus-lib-global.h"

class CORE_MODBUS_LIB_EXPORT MockModbusClient : public ModbusClient {
    Q_OBJECT
public:
    explicit MockModbusClient(QWidget *parent = nullptr);

    // Переопределяем ключевые методы, используемые в тестируемом коде
    void sendSingleRequest(int serverAddress, QModbusDataUnit mdu);
    void on_write(int serverAddress, QModbusDataUnit mdu);
    void on_read(int serverAddress, QModbusDataUnit mdu);
    void writeMultipleRegisters(int serverAddress, int startAddress,
                                const QVector<quint16>& values);
    void setRegister(int serverAddress, const QString &reg_name, double value);

    // Методы настройки сценария
    void setReadResponse(int serverAddress, const QModbusDataUnit &response);
    QVector<quint16> lastWrittenData(int serverAddress) const;

    void simulateServerHealth(int serverAddress, bool healthy) {emit serverHealthChanged(serverAddress, healthy);}
    void simulateConnection() {emit host_connected();}
    void simulateDisconnection() {emit host_disconnected();}

    // Переопределяем, чтобы избежать реального подключения
    void connectToHost() { emit host_connected(); }
    void disconnectFromHost() { emit host_disconnected(); }
    bool isConnected() const { return m_connected; }
    void setConnected(bool c) { m_connected = c; }

private:
    std::map<int, QModbusDataUnit> m_responses;
    std::map<int, QVector<quint16>> m_writtenData;
    bool m_connected = true;
};
#endif // MOCKMODBUSCLIENT_H
