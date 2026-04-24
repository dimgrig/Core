#ifndef MODBUSBASE_H
#define MODBUSBASE_H

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QModbusDevice>
#include <QObject>
#include <QTimer>
#include <QVariant>

#include <QModbusDataUnit>

#include "CoreConfig.h"

#include "ModbusSettings.h"

#include "core-modbus-lib-global.h"

ENUM_MACRO11(ModbusRegisterDataType, ModbusRegisterDataType_Undefined, ModbusRegisterDataType_bool,
             ModbusRegisterDataType_ushort, ModbusRegisterDataType_short,
             ModbusRegisterDataType_uint, ModbusRegisterDataType_int,
             ModbusRegisterDataType_ulonglong, ModbusRegisterDataType_longlong,
             ModbusRegisterDataType_float, ModbusRegisterDataType_double,
             ModbusRegisterDataType_string,
             ModbusRegisterDataType_Undefined, Bool, UShort, Short, UInt32, Int32, ULongLong, LongLong, Float, Double, String,
             Тип не определен,                 Bool, UShort, Short, UInt32, Int32, ULongLong, LongLong, Float, Double, String,
             0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

static QStringList ModbusRegisterDataTypes = {ModbusRegisterDataType_to_text(ModbusRegisterDataType_bool),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_ushort),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_short),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_uint),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_int),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_ulonglong),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_longlong),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_float),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_double),
                                              ModbusRegisterDataType_to_text(ModbusRegisterDataType_string)};

ENUM_MACRO5(ModbusRegisterType, ModbusRegisterType_Undefined, ModbusRegisterType_DiscreteInput,
            ModbusRegisterType_Coil, ModbusRegisterType_InputRegister, ModbusRegisterType_HoldingRegister,
            Invalid, DiscreteInput, Coil, InputRegister, HoldingRegister,
            Invalid, DiscreteInput, Coil, InputRegister, HoldingRegister,
            0, 1, 2, 3, 4);

static QStringList ModbusRegisterTypes = {ModbusRegisterType_to_text(ModbusRegisterType_DiscreteInput),
                                          ModbusRegisterType_to_text(ModbusRegisterType_Coil),
                                          ModbusRegisterType_to_text(ModbusRegisterType_InputRegister),
                                          ModbusRegisterType_to_text(ModbusRegisterType_HoldingRegister)};

class CORE_MODBUS_LIB_EXPORT ModbusBase : public QWidget {
Q_OBJECT
public:
    ModbusBase(QWidget *parent = nullptr, QString config = "", ModbusSettings modbus_settings = ModbusSettings());

    virtual void updateDeviceMap() = 0;
    virtual void updatePollingGroups() = 0;

    bool isConnected() const;

    QString get_config() const {return _config;};
    int getServerAddress() const {return _modbus_settings.MODBUS_ADDRESS;};

    BytesFormat modbus_format() const {return BytesFormat_from_text(this->_modbus_settings.MODBUS_FORMAT.toStdString().c_str());};

    QModbusDataUnit::RegisterType getRegisterType(int serverAddress, const QString regName) const;

    void set_modbus_settings(ModbusSettings modbus_settings);
    void set_modbus_config(QString config);

    QModbusDataUnitMap get_reg(int serverAddress) const;
    std::vector<QModbusDataUnitMap> get_reg_v(int serverAddress) const;

    std::map<int, std::map<QString, QModbusDataUnit>> get_mdum() const {return _mdum;};
    std::map<int, std::vector<std::pair<QString, QModbusDataUnit>>> get_sort_by_start_address_mdum_v() const;
    std::map<int, std::map<QString, QString>> get_reg_data_type_m() const {return _reg_data_type_m;};

    void set_mdu_reg_data_type(int serverAddress, std::map<int, std::map<QString, QModbusDataUnit>> mdum, std::map<int, std::map<QString, QString>> reg_data_type_m);
    void add_mdu_reg_data_type(int serverAddress, QString reg_name, QModbusDataUnit mdu, QString reg_data_type);
signals:
    void host_error(QString error);
    void host_connected();
    void host_connecting();
    void host_disconnected();

    void on_connected();
    void on_connecting();
    void on_disconnected();

    void on_state_changed(int state);
    void state_changed(QString state);

    void config_changed(QString config);
public slots:
    QModbusDevice::State state() const {return this->device()->state();};

    virtual std::shared_ptr<QModbusDevice> device() const = 0;

    virtual void on_connect_type(int index) = 0;
    virtual void on_connect() = 0;
    virtual void disconnectFromHost() = 0;

    void connectToHost() {this->on_connect();};

    bool on_write_json(const QString& config);
protected slots:
    void checkSerialPorts();
protected:
    void setupDeviceData();

    std::map<int, std::map<QString, QModbusDataUnit>> update_mdu(int serverAddress,
                                                  QModbusDataUnit::RegisterType table,
                                                  unsigned int address,
                                                  unsigned int size,
                                                  std::map<int, std::map<QString, QModbusDataUnit>> &mdum);

    void on_read_json(QString config);

    bool isSerialPortAvailable(const QString &portName);

    QString _config = "";

    std::map<int, QModbusDataUnitMap> _reg; //no blanc space between registers
    std::map<int, std::vector<QModbusDataUnitMap>> _reg_v; //vector for splitting blanc space between registers
    std::map<int, std::map<QString, QModbusDataUnit>> _mdum;
    std::map<int, std::map<QString, QString>> _reg_data_type_m; //map reg_name to type

    ModbusSettings _modbus_settings;
    ConnectionType _connection_type = ConnectionTypeTCP;

    QTimer *_portCheckTimer;  // Таймер для проверки портов
    QString _lastKnownPortState;  // Для отслеживания изменений состояния порта
};

#endif // MODBUSBASE_H
