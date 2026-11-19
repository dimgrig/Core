#ifndef WASMMODBUSSTUBS_H
#define WASMMODBUSSTUBS_H

#if defined(__EMSCRIPTEN__) && !defined(CORE_ENABLE_SERIAL)

#include <QVector>
#include <QString>
#include <QMap>
#include <QByteArray>
#include <QObject>

// ============================================================================
//  Stub для QModbusDataUnit
// ============================================================================
class QModbusDataUnit {
public:
    enum RegisterType {
        Invalid = -1,
        DiscreteInputs = 0,
        Coils = 1,
        InputRegisters = 2,
        HoldingRegisters = 3
    };

    QModbusDataUnit() : m_registerType(Invalid), m_startAddress(0), m_valueCount(0), m_exceptionCode(0) {}
    QModbusDataUnit(RegisterType type, int startAddr, quint16 count)
        : m_registerType(type), m_startAddress(startAddr), m_valueCount(count), m_exceptionCode(0) {}
    QModbusDataUnit(RegisterType type, int startAddr, const QVector<quint16>& values)
        : m_registerType(type), m_startAddress(startAddr), m_valueCount(values.size()), m_values(values), m_exceptionCode(0) {}

    RegisterType registerType() const { return m_registerType; }
    void setRegisterType(RegisterType type) { m_registerType = type; }

    int startAddress() const { return m_startAddress; }
    void setStartAddress(int addr) { m_startAddress = addr; }

    quint16 valueCount() const { return m_valueCount; }
    void setValueCount(quint16 count) { m_valueCount = count; }

    QVector<quint16> values() const { return m_values; }
    void setValues(const QVector<quint16>& vals) {
        m_values = vals;
        m_valueCount = vals.size();
    }

    void setValue(int index, quint16 value) {
        if (index >= 0 && index < m_values.size()) {
            m_values[index] = value;
        }
    }

    quint16 value(int index) const {
        if (index >= 0 && index < m_values.size()) {
            return m_values.at(index);
        }
        return 0;
    }

    bool isValid() const { return m_registerType != Invalid; }

    // Код исключения Modbus (для обработки ProtocolError)
    quint8 exceptionCode() const { return m_exceptionCode; }
    void setExceptionCode(quint8 code) { m_exceptionCode = code; }
private:
    RegisterType m_registerType;
    int m_startAddress;
    quint16 m_valueCount;
    QVector<quint16> m_values;
    quint8 m_exceptionCode;
};

// Typedef для QModbusDataUnitMap
typedef QMap<QModbusDataUnit::RegisterType, QModbusDataUnit> QModbusDataUnitMap;

// ============================================================================
//  Stub для QModbusDevice
// ============================================================================
class QModbusDevice : public QObject {
    Q_OBJECT
public:
    enum State {
        UnconnectedState = 0,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    enum Error {
        NoError = 0,
        ReadError,
        WriteError,
        ConnectionError,
        ProtocolError,
        UnknownError
    };

    enum ConnectionParameter {
        SerialPortNameParameter,
        SerialBaudRateParameter,
        SerialDataBitsParameter,
        SerialStopBitsParameter,
        SerialParityParameter,
        NetworkAddressParameter,
        NetworkPortParameter
    };

    QModbusDevice(QObject *parent = nullptr) : QObject(parent), m_state(UnconnectedState) {}
    virtual ~QModbusDevice() {}

    State state() const { return m_state; }
    void setState(State state) { m_state = state; }

    virtual bool connectDevice() { return false; }
    virtual void disconnectDevice() {}

    QString errorString() const { return m_errorString; }

    void setConnectionParameter(ConnectionParameter param, const QVariant &value) {
        Q_UNUSED(param); Q_UNUSED(value);
    }

    void setTimeout(int msecs) { Q_UNUSED(msecs); }
    void setNumberOfRetries(int retries) { Q_UNUSED(retries); }

signals:
    void stateChanged(QModbusDevice::State state);
    void errorOccurred(QModbusDevice::Error error);

protected:
    State m_state;
    QString m_errorString;
};

// ============================================================================
//  Stub для QModbusReply
// ============================================================================
class QModbusReply : public QObject {
    Q_OBJECT
public:
    QModbusReply(QObject *parent = nullptr) : QObject(parent), m_finished(false), m_error(QModbusDevice::NoError) {}

    bool isFinished() const { return m_finished; }
    void setFinished(bool finished) { m_finished = finished; }

    QModbusDevice::Error error() const { return m_error; }
    void setError(QModbusDevice::Error error) { m_error = error; }

    QString errorString() const { return m_errorString; }
    void setErrorString(const QString &errorString) { m_errorString = errorString; }

    int serverAddress() const { return m_serverAddress; }
    void setServerAddress(int address) { m_serverAddress = address; }

    QModbusDataUnit result() const { return m_result; }
    void setResult(const QModbusDataUnit &result) { m_result = result; }

    QModbusDataUnit rawResult() const { return m_result; }

signals:
    void finished();

private:
    bool m_finished;
    QModbusDevice::Error m_error;
    QString m_errorString;
    int m_serverAddress = 0;
    QModbusDataUnit m_result;
};

// ============================================================================
//  Stub для QModbusClient
// ============================================================================
class QModbusClient : public QModbusDevice {
    Q_OBJECT
public:
    QModbusClient(QObject *parent = nullptr) : QModbusDevice(parent) {}
    virtual ~QModbusClient() {}

    virtual QModbusReply *sendReadRequest(const QModbusDataUnit &data, int slaveAddress) {
        Q_UNUSED(data); Q_UNUSED(slaveAddress);
        return nullptr;
    }

    virtual QModbusReply *sendWriteRequest(const QModbusDataUnit &data, int slaveAddress) {
        Q_UNUSED(data); Q_UNUSED(slaveAddress);
        return nullptr;
    }
};

// ============================================================================
//  Stub для QModbusServer
// ============================================================================
class QModbusServer : public QModbusDevice {
    Q_OBJECT
public:
    QModbusServer(QObject *parent = nullptr) : QModbusDevice(parent) {}
    virtual ~QModbusServer() {}

    void setServerAddress(int address) { Q_UNUSED(address); }

    bool setData(const QModbusDataUnit &data) {
        Q_UNUSED(data);
        return false;
    }

    bool setData(QModbusDataUnit::RegisterType table, int address, quint16 value) {
        Q_UNUSED(table); Q_UNUSED(address); Q_UNUSED(value);
        return false;
    }

    bool data(QModbusDataUnit::RegisterType table, int address, quint16 *data) const {
        Q_UNUSED(table); Q_UNUSED(address);
        if (data) *data = 0;
        return false;
    }

    bool data(QModbusDataUnit *unit) const {
        Q_UNUSED(unit);
        return false;
    }
    void setMap(const QModbusDataUnitMap &map) { Q_UNUSED(map); }

signals:
    void dataWritten(QModbusDataUnit::RegisterType table, int address, int size);
};

// ============================================================================
//  Stub для QModbusPdu и связанных классов (для ModbusMultiAddressServer)
// ============================================================================
class QModbusPdu {
public:
    enum FunctionCode {
        Invalid = 0x00,
        ReadCoils = 0x01,
        ReadDiscreteInputs = 0x02,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        ReadExceptionStatus = 0x07,
        Diagnostics = 0x08,
        ReportServerId = 0x11,
        WriteMultipleCoils = 0x0F,
        WriteMultipleRegisters = 0x10,
        ReadWriteMultipleRegisters = 0x17,
        ReadFifoQueue = 0x18
    };

    QModbusPdu() : m_functionCode(Invalid) {}
    QModbusPdu(FunctionCode code, const QByteArray &data = QByteArray())
        : m_functionCode(code), m_data(data) {}

    FunctionCode functionCode() const { return m_functionCode; }
    QByteArray data() const { return m_data; }
    bool isValid() const { return m_functionCode != Invalid; }

protected:
    FunctionCode m_functionCode;
    QByteArray m_data;
};

class QModbusRequest : public QModbusPdu {
public:
    QModbusRequest() : QModbusPdu() {}
    QModbusRequest(FunctionCode code, const QByteArray &data = QByteArray())
        : QModbusPdu(code, data) {}
};

class QModbusResponse : public QModbusPdu {
public:
    QModbusResponse() : QModbusPdu() {}
    QModbusResponse(FunctionCode code, const QByteArray &data = QByteArray())
        : QModbusPdu(code, data) {}
};

class QModbusExceptionResponse : public QModbusResponse {
public:
    enum ExceptionCode {
        IllegalFunction = 0x01,
        IllegalDataAddress = 0x02,
        IllegalDataValue = 0x03,
        ServerDeviceFailure = 0x04
    };

    QModbusExceptionResponse() : QModbusResponse() {}
    QModbusExceptionResponse(FunctionCode code, ExceptionCode exceptionCode)
        : QModbusResponse(code, QByteArray(1, static_cast<char>(exceptionCode))) {}
};

// ============================================================================
//  Stub для QModbusTcpClient (для Wasm — модуль QtSerialBus отсутствует)
//  В будущем будет заменён на WebSocket-реализацию через мост
// ============================================================================
class QModbusTcpClient : public QModbusClient {
    Q_OBJECT
public:
    QModbusTcpClient(QObject *parent = nullptr) : QModbusClient(parent) {}
    virtual ~QModbusTcpClient() {}

    bool connectDevice() override {
        // Заглушка: в Wasm реальное TCP-подключение недоступно
        // Будет заменено на WebSocket-транспорт на этапе 2
        m_errorString = "Wasm stub: TCP Modbus not available. Use Modbus Bridge.";
        return false;
    }

    void disconnectDevice() override {
        m_state = UnconnectedState;
        emit stateChanged(m_state);
    }

    QModbusReply *sendReadRequest(const QModbusDataUnit &data, int slaveAddress) override {
        Q_UNUSED(data); Q_UNUSED(slaveAddress);
        // Возвращаем nullptr — вызывающий код обрабатывает это как ошибку
        return nullptr;
    }

    QModbusReply *sendWriteRequest(const QModbusDataUnit &data, int slaveAddress) override {
        Q_UNUSED(data); Q_UNUSED(slaveAddress);
        return nullptr;
    }
};

// ============================================================================
//  Stub для QModbusTcpServer (для Wasm — модуль QtSerialBus отсутствует)
//  В будущем будет заменён на WebSocket-реализацию через мост
// ============================================================================
class QModbusTcpServer : public QModbusServer {
    Q_OBJECT
public:
    QModbusTcpServer(QObject *parent = nullptr) : QModbusServer(parent) {}
    virtual ~QModbusTcpServer() {}

    bool connectDevice() override {
        m_errorString = "Wasm stub: TCP Modbus server not available. Use Modbus Bridge.";
        return false;
    }

    void disconnectDevice() override {
        m_state = UnconnectedState;
        emit stateChanged(m_state);
    }

    // Методы для работы с данными (заглушки)
    bool setData(const QModbusDataUnit &data) {
        Q_UNUSED(data);
        return false;
    }

    bool setData(QModbusDataUnit::RegisterType table, int address, quint16 value) {
        Q_UNUSED(table); Q_UNUSED(address); Q_UNUSED(value);
        return false;
    }

    bool data(QModbusDataUnit::RegisterType table, int address, quint16 *data) const {
        Q_UNUSED(table); Q_UNUSED(address);
        if (data) *data = 0;
        return false;
    }

    void setMap(const QModbusDataUnitMap &map) {
        Q_UNUSED(map);
    }
};

// ============================================================================
//  Stub для QModbusRtuSerialClient/Server (если вдруг потребуется)
//  В Wasm они не используются, но могут понадобиться для компиляции
// ============================================================================
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
class QModbusRtuSerialClient : public QModbusClient {
    Q_OBJECT
public:
    QModbusRtuSerialClient(QObject *parent = nullptr) : QModbusClient(parent) {}
    bool connectDevice() override { return false; }
    void disconnectDevice() override { m_state = UnconnectedState; }
    QModbusReply *sendReadRequest(const QModbusDataUnit &, int) override { return nullptr; }
    QModbusReply *sendWriteRequest(const QModbusDataUnit &, int) override { return nullptr; }
};

class QModbusRtuSerialServer : public QModbusServer {
    Q_OBJECT
public:
    QModbusRtuSerialServer(QObject *parent = nullptr) : QModbusServer(parent) {}
    bool connectDevice() override { return false; }
    void disconnectDevice() override { m_state = UnconnectedState; }
};
#else
class QModbusRtuSerialMaster : public QModbusClient {
    Q_OBJECT
public:
    QModbusRtuSerialMaster(QObject *parent = nullptr) : QModbusClient(parent) {}
    bool connectDevice() override { return false; }
    void disconnectDevice() override { m_state = UnconnectedState; }
    QModbusReply *sendReadRequest(const QModbusDataUnit &, int) override { return nullptr; }
    QModbusReply *sendWriteRequest(const QModbusDataUnit &, int) override { return nullptr; }
};

class QModbusRtuSerialSlave : public QModbusServer {
    Q_OBJECT
public:
    QModbusRtuSerialSlave(QObject *parent = nullptr) : QModbusServer(parent) {}
    bool connectDevice() override { return false; }
    void disconnectDevice() override { m_state = UnconnectedState; }
};
#endif

#endif // __EMSCRIPTEN__ && !CORE_ENABLE_SERIAL

#endif // WASMMODBUSSTUBS_H
