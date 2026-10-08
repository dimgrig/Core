#ifndef MODBUSMULTIADDRESSSERVER_H
#define MODBUSMULTIADDRESSSERVER_H

#include <QSerialPort>
#include <QTimer>
#include <QByteArray>
#include <QList>
#include <map>

#include <QModbusDataUnit>
#include <QModbusPdu>

#include "ModbusBase.h"
#include "core-modbus-lib-global.h"

Q_DECLARE_METATYPE(ModbusSettings)

class CORE_MODBUS_LIB_EXPORT ModbusMultiAddressServer : public ModbusBase {
    Q_OBJECT
public:
    explicit ModbusMultiAddressServer(QWidget *parent = nullptr,
                                      QString config = "",
                                      ModbusSettings modbus_settings = ModbusSettings());
    ~ModbusMultiAddressServer() override;

    void updateDeviceMap() override;
    void updatePollingGroups() override;

    std::shared_ptr<QModbusDevice> device() const override { return nullptr; }

    // ---- Мультиадресный API ----
    QList<int> slaveAddresses() const;
    bool hasSlave(int address) const { return _mdum.count(address) > 0; }

    BytesFormat slaveFormat(int address) const;

    // Карта регистров конкретного slave (для UI)
    std::map<QString, QModbusDataUnit> slaveMdum(int address) const;
    std::map<QString, QString>         slaveRegTypes(int address) const;

    ModbusSettings transportSettings() const { return _transportSettings; }
    ModbusSettings slaveSettings(int address) const;

signals:
    void read_ready(int serverAddress, QString reg_name, QModbusDataUnit mdu);

    void slaveAdded(int address);
    void slaveRemoved(int address);
    void slaveConfigLoaded(int address, bool ok);
    void slaveSettingsChanged(int address);

public slots:
    void on_connect_type(int index) override;
    void on_connect() override;
    void disconnectFromHost() override;

    // Управление slave'ами
    void addSlave(int address, const ModbusSettings& slaveSettings);
    void removeSlave(int address);
    void setSlaveSettings(int address, const ModbusSettings& slaveSettings);
    void setSlaveConfig(int address, const QString& jsonPath);
    void reloadSlaveConfig(int address);

    // Транспорт — берётся из "мастер" таба
    void setTransportSettings(const ModbusSettings& settings);

    // Запись регистров из UI (как у ModbusServer)
    void setRegister(int serverAddress, const QString &reg_name, double value);
    void setRegisterString(int serverAddress, const QString &reg_name, const QString &value);
    void setRegister(int serverAddress, QModbusDataUnit::RegisterType table,
                     const QString &reg_name, double value);
    void setRegisterString(int serverAddress, QModbusDataUnit::RegisterType table,
                           const QString &reg_name, const QString &value);

private slots:
    void onSerialReadyRead();
    void onSerialError(QSerialPort::SerialPortError err);
    void onInterFrameTimeout();
    void onReconnectTimer();

private:
    void processFrame(const QByteArray &frame);
    void sendResponse(quint8 address, const QModbusPdu &pdu);
    void scheduleReconnect();

    QModbusResponse dispatch(quint8 address, const QModbusPdu &request);

    QModbusResponse handleReadCoils(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleReadDiscreteInputs(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleReadHoldingRegisters(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleReadInputRegisters(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleWriteSingleCoil(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleWriteSingleRegister(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleWriteMultipleCoils(quint8 addr, const QModbusPdu &req);
    QModbusResponse handleWriteMultipleRegisters(quint8 addr, const QModbusPdu &req);

    bool readValues(int addr, QModbusDataUnit::RegisterType type,
                    quint16 start, quint16 count, QVector<quint16> &outValues) const;
    bool writeValues(int addr, QModbusDataUnit::RegisterType type,
                     quint16 start, const QVector<quint16> &values);

    QSerialPort *_port = nullptr;
    QByteArray   _rxBuffer;
    QTimer      *_interFrameTimer = nullptr;
    QTimer      *_reconnectTimer = nullptr;
    int          _failedAttempts = 0;
    int          _interFrameDelayMs = 4;

    ModbusSettings _transportSettings;                 // настройки порта (из первого таба)
    std::map<int, ModbusSettings> _slaveSettings;      // настройки каждого slave
    std::map<int, QString>        _slaveConfigPaths;   // путь к JSON каждого slave

    static constexpr int MAX_FAILED_ATTEMPTS = 3;
};

#endif // MODBUSMULTIADDRESSSERVER_H
