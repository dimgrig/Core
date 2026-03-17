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

#include "enum_macro.h"

#include "ModbusSettings.h"

#include "core-modbus-lib-global.h"

ENUM_MACRO2(ModbusConnection, ModbusSerial, ModbusTCP,
            Serial, TCP,
            Serial, TCP,
            0, 1);

ENUM_MACRO8(ModbusBaudRate, ModbusBaud1200 = 1200, ModbusBaud2400 = 2400, ModbusBaud4800 = 4800, ModbusBaud9600 = 9600,
            ModbusBaud19200 = 19200, ModbusBaud38400 = 38400, ModbusBaud57600 = 57600, ModbusBaud115200 = 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200);

ENUM_MACRO4(ModbusDataBits, ModbusDataBits5 = 5, ModbusDataBits6 = 6, ModbusDataBits7 = 7, ModbusDataBits8 = 8,
            5, 6, 7, 8,
            5, 6, 7, 8,
            5, 6, 7, 8);

ENUM_MACRO3(ModbusStopBits, ModbusStopBits_OneStop = 1, ModbusStopBits_TwoStop = 2, ModbusStopBits_OneAndHalfStop = 3,
            1, 2, 1.5,
            1, 2, 1.5,
            1, 2, 3);

ENUM_MACRO5(ModbusParity, ModbusParity_NoParity = 0, ModbusParity_EvenParity = 2, ModbusParity_OddParity = 3,
            ModbusParity_SpaceParity = 4, ModbusParity_MarkParity = 5,
            NoParity, EvenParity, OddParity, SpaceParity, MarkParity,
            NoParity, EvenParity, OddParity, SpaceParity, MarkParity,
            0, 2, 3, 4, 5);

// big-endian
// big-endian слова, но байты в словах переставлены
// little-endian слова, но байты в словах не переставлены
// little-endian
ENUM_MACRO4(ModbusFormat, BigEndian, BigEndianByteSwap, LittleEndianByteSwap, LittleEndian,
            ABCD, BADC, CDAB, DCBA,
            ABCD, BADC, CDAB, DCBA,
            0, 1, 2, 3);

static QStringList ModbusConnection_sl = {{ModbusConnection_to_text(ModbusSerial),
                                           ModbusConnection_to_text(ModbusTCP)}};
static QStringList ModbusBaudRate_sl = {{ModbusBaudRate_to_text(ModbusBaud1200),
                                         ModbusBaudRate_to_text(ModbusBaud2400),
                                         ModbusBaudRate_to_text(ModbusBaud4800),
                                         ModbusBaudRate_to_text(ModbusBaud9600),
                                         ModbusBaudRate_to_text(ModbusBaud19200),
                                         ModbusBaudRate_to_text(ModbusBaud38400),
                                         ModbusBaudRate_to_text(ModbusBaud57600),
                                         ModbusBaudRate_to_text(ModbusBaud115200)}};
static QStringList ModbusDataBits_sl = {{ModbusDataBits_to_text(ModbusDataBits5),
                                         ModbusDataBits_to_text(ModbusDataBits6),
                                         ModbusDataBits_to_text(ModbusDataBits7),
                                         ModbusDataBits_to_text(ModbusDataBits8)}};
static QStringList ModbusStopBits_sl = {{ModbusStopBits_to_text(ModbusStopBits_OneStop),
                                         ModbusStopBits_to_text(ModbusStopBits_TwoStop),
                                         ModbusStopBits_to_text(ModbusStopBits_OneAndHalfStop)}};
static QStringList ModbusParity_sl = {{ModbusParity_to_text(ModbusParity_NoParity),
                                       ModbusParity_to_text(ModbusParity_EvenParity),
                                       ModbusParity_to_text(ModbusParity_OddParity),
                                       ModbusParity_to_text(ModbusParity_SpaceParity),
                                       ModbusParity_to_text(ModbusParity_MarkParity)}};
static QStringList ModbusFormat_sl = {{ModbusFormat_to_text(BigEndian),
                                       ModbusFormat_to_text(BigEndianByteSwap),
                                       ModbusFormat_to_text(LittleEndianByteSwap),
                                       ModbusFormat_to_text(LittleEndian)}};

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

    bool isConnected() const;

    QString get_config() const {return _config;};

    ModbusFormat modbus_format() const {return ModbusFormat_from_text(this->_modbus_settings.MODBUS_FORMAT.toStdString().c_str());};

    QModbusDataUnit::RegisterType getRegisterType(const QString regName) const;

    void set_modbus_settings(ModbusSettings modbus_settings);
    void set_modbus_config(QString config);

    std::map<QString, QModbusDataUnit> get_mdum() const {return _mdum;};
    QModbusDataUnitMap get_reg() const {return _reg;};
    std::vector<QModbusDataUnitMap> get_reg_v() const {return _reg_v;};

    std::vector<std::pair<QString, QModbusDataUnit>> get_sort_by_start_address_mdum_v() const;
    std::map<QString, QString> get_reg_data_type_m() const {return _reg_data_type_m;};

    void set_mdu_reg_data_type(std::map<QString, QModbusDataUnit> mdum, std::map<QString, QString> reg_data_type_m);
    void add_mdu_reg_data_type(QString reg_name, QModbusDataUnit mdu, QString reg_data_type);
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

    std::map<QString, QModbusDataUnit> update_mdu(QModbusDataUnit::RegisterType table, unsigned int address, unsigned int size, std::map<QString, QModbusDataUnit> &mdum);

    void on_read_json(QString config);

    bool isSerialPortAvailable(const QString &portName);

    QString _config = "";

    std::map<QString, QModbusDataUnit> _mdum;
    QModbusDataUnitMap _reg; //no blanc space between registers
    std::vector<QModbusDataUnitMap> _reg_v; //vector for splitting blanc space between registers
    std::map<QString, QString> _reg_data_type_m; //map reg_name to type

    ModbusSettings _modbus_settings;
    ModbusConnection _connection_type = ModbusTCP;

    QTimer *_portCheckTimer;  // Таймер для проверки портов
    QString _lastKnownPortState;  // Для отслеживания изменений состояния порта
};

#endif // MODBUSBASE_H
