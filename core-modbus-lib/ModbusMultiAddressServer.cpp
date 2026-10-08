#include "ModbusMultiAddressServer.h"

#include <QSerialPort>
#include <QModbusRequest>
#include <QModbusResponse>
#include <QModbusExceptionResponse>
#include <cmath>

#include "Logger.h"
#include "Categories.h"
#include "JournalAccess.h"
#include "ModbusDataConverter.h"
#include "ModbusSettings.h"
#include "ModbusRegisterSerializer.h"

// ---------- CRC16 Modbus (poly 0xA001, init 0xFFFF) ----------
static quint16 crc16_modbus(const quint8 *data, int len) {
    quint16 crc = 0xFFFF;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
    }
    return crc;
}

// t3.5 = 3.5 * 11 bit / baud, минимум 1 мс (гранулярность QTimer)
static int computeInterFrameDelayMs(int baudrate) {
    if (baudrate <= 0) return 20;
    double t = 3.5 * 11.0 * 1000.0 / baudrate;
    // USB-адаптеры буферизуют данные кусками; 5 мс — практический минимум
    return qMax(20, static_cast<int>(std::ceil(t)));
}

// ============================================================================
//  Конструктор / деструктор
// ============================================================================

ModbusMultiAddressServer::ModbusMultiAddressServer(QWidget *parent,
                                                   QString config,
                                                   ModbusSettings s)
    : ModbusBase(parent, config, s)
    , _transportSettings(s)
{
    _interFrameDelayMs = computeInterFrameDelayMs(s.MODBUS_BAUDRATE);
}

ModbusMultiAddressServer::~ModbusMultiAddressServer() {
    if (_port && _port->isOpen())
        _port->close();
}

void ModbusMultiAddressServer::updateDeviceMap()     {}
void ModbusMultiAddressServer::updatePollingGroups() {}

// ============================================================================
//  Транспорт: открытие порта
// ============================================================================

void ModbusMultiAddressServer::on_connect_type(int index) {
    Q_UNUSED(index);
    _connection_type = ConnectionTypeSerial;   // только RTU
}

void ModbusMultiAddressServer::on_connect() {
    if (!_port) {
        _port = new QSerialPort(this);
        connect(_port, &QSerialPort::readyRead,
                this, &ModbusMultiAddressServer::onSerialReadyRead);
        connect(_port, &QSerialPort::errorOccurred,
                this, &ModbusMultiAddressServer::onSerialError);

        _interFrameTimer = new QTimer(this);
        _interFrameTimer->setSingleShot(true);
        connect(_interFrameTimer, &QTimer::timeout,
                this, &ModbusMultiAddressServer::onInterFrameTimeout);

        _reconnectTimer = new QTimer(this);
        _reconnectTimer->setSingleShot(true);
        connect(_reconnectTimer, &QTimer::timeout,
                this, &ModbusMultiAddressServer::onReconnectTimer);
    }

    if (_port->isOpen())
        return;

    const QString portName = _transportSettings.MODBUS_SERIAL_PORT;
    if (!isSerialPortAvailable(portName)) {
        emit host_error(tr("Serial port %1 not found").arg(portName));
        scheduleReconnect();
        return;
    }

    _port->setPortName(portName);
    _port->setBaudRate(_transportSettings.MODBUS_BAUDRATE);
    _port->setDataBits(static_cast<QSerialPort::DataBits>(_transportSettings.MODBUS_DATA_BITS));
    _port->setStopBits(static_cast<QSerialPort::StopBits>(_transportSettings.MODBUS_STOP_BITS));
    _port->setParity(static_cast<QSerialPort::Parity>(
        SerialParity_from_text(_transportSettings.MODBUS_PARITY.toStdString().c_str())));
    _port->setFlowControl(QSerialPort::NoFlowControl);

    if (!_port->open(QIODevice::ReadWrite)) {
        emit host_error(_port->errorString());
        scheduleReconnect();
        return;
    }

    _failedAttempts = 0;
    _interFrameDelayMs = computeInterFrameDelayMs(_transportSettings.MODBUS_BAUDRATE);

    // Готовим _reg / _reg_v по всем адресам сразу
    setupDeviceData();

    _localState = QModbusDevice::ConnectedState;
    emit state_changed(ModbusSettingsConverter::modbus_state_to_str(_localState));
    emit on_state_changed(static_cast<int>(_localState));
    emit host_connected();
    emit on_connected();

    qCDebug(logDebug) << "ModbusMultiAddressServer connected on" << portName
                      << "slaves:" << _mdum.size();
}

void ModbusMultiAddressServer::disconnectFromHost() {
    if (_port && _port->isOpen())
        _port->close();
    if (_reconnectTimer) _reconnectTimer->stop();
    if (_interFrameTimer) _interFrameTimer->stop();

    _localState = QModbusDevice::UnconnectedState;
    emit state_changed(ModbusSettingsConverter::modbus_state_to_str(_localState));
    emit on_state_changed(static_cast<int>(_localState));
    emit host_disconnected();
    emit on_disconnected();
}

void ModbusMultiAddressServer::scheduleReconnect() {
    int delay;
    if (_failedAttempts >= MAX_FAILED_ATTEMPTS)
        delay = 5000;
    else
        delay = qMin(5000, 1000 * (1 << qMax(0, _failedAttempts)));

    _reconnectTimer->start(delay);
}

void ModbusMultiAddressServer::onReconnectTimer() {
    ++_failedAttempts;
    on_connect();
}

// ============================================================================
//  Транспорт: приём и разбор кадра
// ============================================================================

void ModbusMultiAddressServer::onSerialReadyRead() {
    if (!_port) return;

    const QByteArray chunk = _port->readAll();
    qCDebug(logDebug) << "RX chunk:" << chunk.size() << "byte(s) ["
                      << chunk.toHex(' ') << "]";

    _rxBuffer.append(chunk);
    // каждый новый кусок байт сбрасывает межкадровую паузу
    _interFrameTimer->start(_interFrameDelayMs);
}

void ModbusMultiAddressServer::onInterFrameTimeout() {
    QByteArray frame = _rxBuffer;
    _rxBuffer.clear();

    qCDebug(logDebug) << "t3.5 fired, buffered frame:" << frame.size()
                      << "byte(s) [" << frame.toHex(' ') << "]";

    if (frame.size() < 4) {
        qCDebug(logDebug) << "  → too short, ignored";
        return;
    }

    const quint16 crc_calc = crc16_modbus(
        reinterpret_cast<const quint8*>(frame.constData()), frame.size() - 2);

    const quint16 crc_recv =
        static_cast<quint8>(frame[frame.size() - 2]) |
        (static_cast<quint8>(frame[frame.size() - 1]) << 8);

    qCDebug(logDebug) << "  CRC calc:" << QString("0x%1").arg(crc_calc, 4, 16, QChar('0'))
                      << "recv:" << QString("0x%1").arg(crc_recv, 4, 16, QChar('0'));

    if (crc_calc != crc_recv) {
        qCDebug(logDebug) << "  → CRC mismatch, frame dropped";
        return;
    }

    processFrame(frame);
}

void ModbusMultiAddressServer::onSerialError(QSerialPort::SerialPortError err) {
    if (err == QSerialPort::NoError) return;
    emit host_error(_port->errorString());

    if (err == QSerialPort::ResourceError || err == QSerialPort::DeviceNotFoundError) {
        if (_port->isOpen()) _port->close();
        _localState = QModbusDevice::UnconnectedState;
        emit state_changed(ModbusSettingsConverter::modbus_state_to_str(_localState));
        emit on_state_changed(static_cast<int>(_localState));
        emit host_disconnected();
        scheduleReconnect();
    }
}

void ModbusMultiAddressServer::processFrame(const QByteArray &frame) {
    const quint8 addr = static_cast<quint8>(frame[0]);
    const quint8 fc   = static_cast<quint8>(frame[1]);

    qCDebug(logDebug) << "processFrame addr:" << addr << "fc:" << fc;

    if (addr == 0) {
        qCDebug(logDebug) << "  → broadcast, ignored";
        return;
    }

    if (!_mdum.count(addr)) {
        qCDebug(logDebug) << "  → no slave for addr" << addr
                          << "known addrs:" << QList<int>(slaveAddresses());
        return;
    }

    const QByteArray pduData = frame.mid(2, frame.size() - 4);
    QModbusRequest req(static_cast<QModbusPdu::FunctionCode>(fc), pduData);

    QModbusResponse resp = dispatch(addr, req);
    if (!resp.isValid()) {
        qCDebug(logDebug) << "  → invalid response, ignored";
        return;
    }

    qCDebug(logDebug) << "  → sending response PDU:"
                      << QByteArray(resp.data()).toHex(' ');
    sendResponse(addr, resp);
}

void ModbusMultiAddressServer::sendResponse(quint8 address, const QModbusPdu &pdu) {
    QByteArray out;
    out.reserve(2 + pdu.data().size() + 2);
    out.append(static_cast<char>(address));
    out.append(static_cast<char>(pdu.functionCode()));   // ← ДОБАВИТЬ ЭТУ СТРОКУ
    out.append(pdu.data());

    const quint16 crc = crc16_modbus(
        reinterpret_cast<const quint8*>(out.constData()), out.size());
    out.append(static_cast<char>(crc & 0xFF));
    out.append(static_cast<char>((crc >> 8) & 0xFF));

    const qint64 written = _port->write(out);
    _port->flush();

    qCDebug(logDebug) << "TX frame:" << out.toHex(' ')
                      << "write():" << written;
}

// ============================================================================
//  Диспетчер
// ============================================================================

QModbusResponse ModbusMultiAddressServer::dispatch(quint8 addr, const QModbusPdu &req) {
    qCDebug(logDebug) << "dispatch addr:" << addr
                      << "fc:" << QString("0x%1").arg(int(req.functionCode()), 2, 16, QChar('0'));

    switch (req.functionCode()) {
    case QModbusPdu::ReadCoils:               return handleReadCoils(addr, req);
    case QModbusPdu::ReadDiscreteInputs:      return handleReadDiscreteInputs(addr, req);
    case QModbusPdu::ReadHoldingRegisters:    return handleReadHoldingRegisters(addr, req);
    case QModbusPdu::ReadInputRegisters:      return handleReadInputRegisters(addr, req);
    case QModbusPdu::WriteSingleCoil:         return handleWriteSingleCoil(addr, req);
    case QModbusPdu::WriteSingleRegister:     return handleWriteSingleRegister(addr, req);
    case QModbusPdu::WriteMultipleCoils:      return handleWriteMultipleCoils(addr, req);
    case QModbusPdu::WriteMultipleRegisters:  return handleWriteMultipleRegisters(addr, req);
    default:
        qCDebug(logDebug) << "  → unsupported function";
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalFunction);
    }
}

// ----------------------------------------------------------------------------
//  Утилиты чтения/записи поверх _mdum[addr]
// ----------------------------------------------------------------------------

bool ModbusMultiAddressServer::readValues(int addr,
                                          QModbusDataUnit::RegisterType type,
                                          quint16 start, quint16 count,
                                          QVector<quint16> &out) const
{
    out.fill(0, count);
    auto itAddr = _mdum.find(addr);
    if (itAddr == _mdum.end()) return false;

    bool any = false;
    for (const auto &kv : itAddr->second) {
        const QModbusDataUnit &u = kv.second;
        if (u.registerType() != type) continue;

        const quint16 uStart = u.startAddress();
        const quint16 uEnd   = uStart + u.valueCount();
        const quint16 rEnd   = start + count;
        if (uEnd <= start || rEnd <= uStart) continue;

        const QVector<quint16> vals = u.values();
        const quint16 lo = qMax(start, uStart);
        const quint16 hi = qMin(rEnd, uEnd);
        for (quint16 i = lo; i < hi; ++i) {
            const int srcIdx = i - uStart;
            out[i - start] = (srcIdx < vals.size()) ? vals[srcIdx] : 0;
        }
        any = true;
    }
    return any;
}

bool ModbusMultiAddressServer::writeValues(int addr,
                                           QModbusDataUnit::RegisterType type,
                                           quint16 start,
                                           const QVector<quint16> &values)
{
    auto itAddr = _mdum.find(addr);
    if (itAddr == _mdum.end()) return false;

    bool any = false;
    const quint16 wEnd = start + values.size();

    for (auto &kv : itAddr->second) {
        QModbusDataUnit &u = kv.second;
        if (u.registerType() != type) continue;

        const quint16 uStart = u.startAddress();
        const quint16 uEnd   = uStart + u.valueCount();
        if (uEnd <= start || wEnd <= uStart) continue;

        QVector<quint16> cur = u.values();
        if (cur.size() < u.valueCount())
            cur.resize(u.valueCount());

        const quint16 lo = qMax(start, uStart);
        const quint16 hi = qMin(wEnd, uEnd);
        for (quint16 i = lo; i < hi; ++i)
            cur[i - uStart] = values[i - start];

        u.setValues(cur);
        any = true;

        // уведомляем UI (тот же сигнал, что у ModbusServer)
        emit read_ready(addr, kv.first, u);
    }

    if (any)
        setupDeviceData();   // пересобрать _reg

    return any;
}

// ============================================================================
//  Обработчики функций Modbus
// ============================================================================

// --- Чтение битов (Coils / DiscreteInputs) ---
static QModbusResponse bitsResponse(QModbusPdu::FunctionCode fc,
                                    const QVector<quint16> &bits)
{
    const quint8 byteCount = static_cast<quint8>((bits.size() + 7) / 8);
    QByteArray payload;
    payload.reserve(1 + byteCount);
    payload.append(static_cast<char>(byteCount));

    QByteArray bytes(byteCount, 0);
    for (int i = 0; i < bits.size(); ++i) {
        if (bits[i])
            bytes[i / 8] = static_cast<char>(static_cast<quint8>(bytes[i / 8]) | (1 << (i % 8)));
    }
    payload.append(bytes);
    return QModbusResponse(fc, payload);
}

QModbusResponse ModbusMultiAddressServer::handleReadCoils(quint8 addr, const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 4)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 count = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);
    if (count < 1 || count > 2000)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> vals;
    if (!readValues(addr, QModbusDataUnit::Coils, start, count, vals))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    QModbusDataUnit unit(QModbusDataUnit::Coils, start, count);
    unit.setValues(vals);
    emit read_ready(addr, QString(), unit);
    return bitsResponse(req.functionCode(), vals);
}

QModbusResponse ModbusMultiAddressServer::handleReadDiscreteInputs(quint8 addr,
                                                                   const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 4)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 count = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);
    if (count < 1 || count > 2000)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> vals;
    if (!readValues(addr, QModbusDataUnit::DiscreteInputs, start, count, vals))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    QModbusDataUnit unit(QModbusDataUnit::DiscreteInputs, start, count);
    unit.setValues(vals);
    emit read_ready(addr, QString(), unit);
    return bitsResponse(req.functionCode(), vals);
}

// --- Чтение регистров ---
static QModbusResponse regsResponse(QModbusPdu::FunctionCode fc,
                                    const QVector<quint16> &regs)
{
    QByteArray payload;
    payload.reserve(1 + regs.size() * 2);
    payload.append(static_cast<char>(regs.size() * 2));
    for (quint16 v : regs) {
        payload.append(static_cast<char>((v >> 8) & 0xFF));
        payload.append(static_cast<char>(v & 0xFF));
    }
    return QModbusResponse(fc, payload);
}

QModbusResponse ModbusMultiAddressServer::handleReadHoldingRegisters(quint8 addr,
                                                                     const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 4)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 count = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);
    if (count < 1 || count > 125)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> vals;
    if (!readValues(addr, QModbusDataUnit::HoldingRegisters, start, count, vals))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, start, count);
    unit.setValues(vals);
    emit read_ready(addr, QString(), unit);
    return regsResponse(req.functionCode(), vals);
}

QModbusResponse ModbusMultiAddressServer::handleReadInputRegisters(quint8 addr,
                                                                   const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 4)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 count = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);
    if (count < 1 || count > 125)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> vals;
    if (!readValues(addr, QModbusDataUnit::InputRegisters, start, count, vals))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    QModbusDataUnit unit(QModbusDataUnit::InputRegisters, start, count);
    unit.setValues(vals);
    emit read_ready(addr, QString(), unit);
    return regsResponse(req.functionCode(), vals);
}

// --- Запись одного бита (0x05) ---
QModbusResponse ModbusMultiAddressServer::handleWriteSingleCoil(quint8 addr,
                                                                const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 4)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 v16   = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);

    if (v16 != 0x0000 && v16 != 0xFF00)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> val{ static_cast<quint16>(v16 == 0xFF00 ? 1 : 0) };
    if (!writeValues(addr, QModbusDataUnit::Coils, start, val))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    return QModbusResponse(req.functionCode(), d);   // эхо запроса
}

// --- Запись одного регистра (0x06) ---
QModbusResponse ModbusMultiAddressServer::handleWriteSingleRegister(quint8 addr,
                                                                    const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 4)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 value = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);

    QVector<quint16> val{ value };
    if (!writeValues(addr, QModbusDataUnit::HoldingRegisters, start, val))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    return QModbusResponse(req.functionCode(), d);   // эхо запроса
}

// --- Запись нескольких битов (0x0F) ---
QModbusResponse ModbusMultiAddressServer::handleWriteMultipleCoils(quint8 addr,
                                                                   const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 6)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start     = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 count     = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);
    const quint8  byteCount = static_cast<quint8>(d[4]);

    if (count < 1 || count > 1968 || byteCount != (count + 7) / 8 ||
        d.size() < 5 + byteCount)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> bits;
    bits.reserve(count);
    for (quint16 i = 0; i < count; ++i) {
        const quint8 byte = static_cast<quint8>(d[5 + i / 8]);
        bits.append((byte >> (i % 8)) & 1);
    }

    if (!writeValues(addr, QModbusDataUnit::Coils, start, bits))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    QByteArray payload;
    payload.append(d[0]); payload.append(d[1]);
    payload.append(d[2]); payload.append(d[3]);
    return QModbusResponse(req.functionCode(), payload);
}

// --- Запись нескольких регистров (0x10) ---
QModbusResponse ModbusMultiAddressServer::handleWriteMultipleRegisters(quint8 addr,
                                                                       const QModbusPdu &req) {
    const QByteArray &d = req.data();
    if (d.size() < 6)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    const quint16 start     = (static_cast<quint8>(d[0]) << 8) | static_cast<quint8>(d[1]);
    const quint16 count     = (static_cast<quint8>(d[2]) << 8) | static_cast<quint8>(d[3]);
    const quint8  byteCount = static_cast<quint8>(d[4]);

    if (count < 1 || count > 123 || byteCount != count * 2 ||
        d.size() < 5 + byteCount)
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataValue);

    QVector<quint16> values;
    values.reserve(count);
    for (quint16 i = 0; i < count; ++i) {
        const quint16 v = (static_cast<quint8>(d[5 + i * 2]) << 8)
        |  static_cast<quint8>(d[5 + i * 2 + 1]);
        values.append(v);
    }

    if (!writeValues(addr, QModbusDataUnit::HoldingRegisters, start, values))
        return QModbusExceptionResponse(req.functionCode(), QModbusPdu::IllegalDataAddress);

    QByteArray payload;
    payload.append(d[0]); payload.append(d[1]);
    payload.append(d[2]); payload.append(d[3]);
    return QModbusResponse(req.functionCode(), payload);
}

// ============================================================================
//  Установка регистров из UI (те же методы, что у ModbusServer)
// ============================================================================

void ModbusMultiAddressServer::setRegister(int serverAddress,
                                           const QString &reg_name, double value) {
    auto regType = getRegisterType(serverAddress, reg_name);
    if (regType != QModbusDataUnit::Invalid)
        setRegister(serverAddress, regType, reg_name, value);
    else if (auto *j = JournalAccess::journal())
        j->warning(Category::Modbus,
                   QString("Не удалось определить тип регистра \"%1\"").arg(reg_name));
}

void ModbusMultiAddressServer::setRegisterString(int serverAddress,
                                                 const QString &reg_name,
                                                 const QString &value) {
    auto regType = getRegisterType(serverAddress, reg_name);
    if (regType != QModbusDataUnit::Invalid)
        setRegisterString(serverAddress, regType, reg_name, value);
    else if (auto *j = JournalAccess::journal())
        j->warning(Category::Modbus,
                   QString("Не удалось определить тип регистра \"%1\"").arg(reg_name));
}

void ModbusMultiAddressServer::setRegister(int serverAddress,
                                           QModbusDataUnit::RegisterType table,
                                           const QString &regName, double value) {
    if (!_mdum.count(serverAddress) || !_mdum[serverAddress].count(regName))
        return;

    QModbusDataUnit &mdu = _mdum[serverAddress][regName];

    switch (table) {
    case QModbusDataUnit::Coils:
    case QModbusDataUnit::DiscreteInputs: {
        mdu.setValue(0, value != 0);
        break;
    }
    case QModbusDataUnit::HoldingRegisters:
    case QModbusDataUnit::InputRegisters: {
        QString dataType = "double";
        auto &typeMap = _reg_data_type_m[serverAddress];
        if (typeMap.count(regName)) dataType = typeMap[regName];

        QVector<quint16> v = ModbusDataConverter::variantToRegisters(
            value, regName, dataType, slaveFormat(serverAddress));
        mdu.setValues(v);
        break;
    }
    default: break;
    }

    setupDeviceData();
}

void ModbusMultiAddressServer::setRegisterString(int serverAddress,
                                                 QModbusDataUnit::RegisterType table,
                                                 const QString &regName,
                                                 const QString &value) {
    if (!_mdum.count(serverAddress) || !_mdum[serverAddress].count(regName))
        return;

    if (table != QModbusDataUnit::HoldingRegisters &&
        table != QModbusDataUnit::InputRegisters) {
        if (auto *j = JournalAccess::journal())
            j->warning(Category::Modbus,
                       QString("Строки не поддерживаются для таблицы %1").arg(table));
        return;
    }

    QModbusDataUnit &mdu = _mdum[serverAddress][regName];
    QString dataType = "double";
    auto &typeMap = _reg_data_type_m[serverAddress];
    if (typeMap.count(regName)) dataType = typeMap[regName];

    QVector<quint16> v = ModbusDataConverter::variantToRegisters(
        value, regName, dataType, slaveFormat(serverAddress), mdu.valueCount());
    mdu.setValues(v);
    setupDeviceData();
}

// ============================================================================
//  Управление slave'ами
// ============================================================================

QList<int> ModbusMultiAddressServer::slaveAddresses() const {
    QList<int> r;
    for (const auto &kv : _mdum) r.append(kv.first);
    return r;
}

BytesFormat ModbusMultiAddressServer::slaveFormat(int address) const {
    auto it = _slaveSettings.find(address);
    if (it != _slaveSettings.end() && !it->second.MODBUS_FORMAT.isEmpty())
        return BytesFormat_from_text(it->second.MODBUS_FORMAT.toStdString().c_str());
    return BytesFormat_from_text(_transportSettings.MODBUS_FORMAT.toStdString().c_str());
}

std::map<QString, QModbusDataUnit> ModbusMultiAddressServer::slaveMdum(int address) const {
    auto it = _mdum.find(address);
    return it != _mdum.end() ? it->second : std::map<QString, QModbusDataUnit>{};
}

std::map<QString, QString> ModbusMultiAddressServer::slaveRegTypes(int address) const {
    auto it = _reg_data_type_m.find(address);
    return it != _reg_data_type_m.end() ? it->second : std::map<QString, QString>{};
}

ModbusSettings ModbusMultiAddressServer::slaveSettings(int address) const {
    auto it = _slaveSettings.find(address);
    return it != _slaveSettings.end() ? it->second : ModbusSettings{};
}

void ModbusMultiAddressServer::setTransportSettings(const ModbusSettings& s) {
    _transportSettings = s;
}

void ModbusMultiAddressServer::addSlave(int address, const ModbusSettings& s) {
    if (address <= 0 || address > 247) {
        emit host_error(tr("Недопустимый адрес slave: %1").arg(address));
        return;
    }
    if (_mdum.count(address)) {
        emit host_error(tr("Slave с адресом %1 уже существует").arg(address));
        return;
    }
    _slaveSettings[address] = s;
    _mdum[address] = {};   // пустая карта
    emit slaveAdded(address);
}

void ModbusMultiAddressServer::removeSlave(int address) {
    if (!_mdum.count(address)) return;

    _slaveSettings.erase(address);
    _slaveConfigPaths.erase(address);
    _mdum.erase(address);
    _reg_data_type_m.erase(address);
    _transactionalGroups.erase(address);
    _reg.erase(address);
    _reg_v.erase(address);

    setupDeviceData();
    emit slaveRemoved(address);
}

void ModbusMultiAddressServer::setSlaveSettings(int address, const ModbusSettings& s) {
    if (!_mdum.count(address)) return;
    _slaveSettings[address] = s;
    emit slaveSettingsChanged(address);
}

void ModbusMultiAddressServer::setSlaveConfig(int address, const QString& jsonPath) {
    if (!_mdum.count(address)) {
        emit slaveConfigLoaded(address, false);
        return;
    }

    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) {
        emit host_error(tr("Не удалось открыть %1").arg(jsonPath));
        emit slaveConfigLoaded(address, false);
        return;
    }
    const QString json = f.readAll();
    f.close();

    auto [mdum, types, groups] = ModbusRegisterSerializer::deserialize(address, json);

    if (!mdum.count(address)) {
        // файл валидный, но без карты для этого адреса — пустая карта
        _mdum[address].clear();
        _reg_data_type_m[address].clear();
        _transactionalGroups[address].clear();
    } else {
        _mdum[address]              = mdum[address];
        _reg_data_type_m[address]   = types[address];
        _transactionalGroups[address] = groups[address];
    }

    _slaveConfigPaths[address] = jsonPath;

    setupDeviceData();
    emit slaveConfigLoaded(address, true);
}

void ModbusMultiAddressServer::reloadSlaveConfig(int address) {
    auto it = _slaveConfigPaths.find(address);
    if (it == _slaveConfigPaths.end() || it->second.isEmpty()) {
        emit slaveConfigLoaded(address, false);
        return;
    }
    setSlaveConfig(address, it->second);
}
