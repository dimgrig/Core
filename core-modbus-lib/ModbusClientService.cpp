#include "ModbusClientService.h"

ModbusClientService::ModbusClientService(ModbusClient* modbusClient, QObject* parent)
    : IModbusService(parent),
    _modbusClient(nullptr)
{
    setModbusClient(modbusClient);
}

ModbusClientService::~ModbusClientService() {
    if (_modbusClient) {
        // не владеем объектом ModbusClient — не удаляем
        disconnect(_modbusClient, nullptr, this, nullptr);
    }
}

void ModbusClientService::setModbusClient(ModbusClient* client) {
    QMutexLocker locker(&_mutex);
    if (_modbusClient) {
        disconnect(_modbusClient, nullptr, this, nullptr);
    }
    _modbusClient = client;
    if (_modbusClient) {
        // ModbusClient в ModbusCore имеет сигнал read_ready(QModbusDataUnit)
        connect(_modbusClient, &ModbusClient::read_ready, this, &ModbusClientService::onReadReady, Qt::QueuedConnection);

        // подключаем базовые события
        connect(_modbusClient, &ModbusBase::host_connected, this, &ModbusClientService::onHostConnected, Qt::QueuedConnection);
        connect(_modbusClient, &ModbusBase::host_disconnected, this, &ModbusClientService::onHostDisconnected, Qt::QueuedConnection);
        connect(_modbusClient, &ModbusBase::host_error, this, [this](const QString& err){
            onHostError(err);
        }, Qt::QueuedConnection);
    }
}

void ModbusClientService::connectToHost() {
    if (!_modbusClient) {
        emit errorOccurred(QStringLiteral("Modbus client not set"));
        return;
    }
    _modbusClient->on_connect(); // существующий метод ModbusClient::on_connect()
}

void ModbusClientService::disconnectFromHost() {
    if (!_modbusClient) return;
    _modbusClient->disconnectFromHost();
}

void ModbusClientService::readRegister(const QString& registerName) {
    QModbusDataUnit mdu;
    if (!findMduByName(registerName, mdu)) {
        emit errorOccurred(QStringLiteral("Register mapping not found: %1").arg(registerName));
        return;
    }
    if (!_modbusClient) {
        emit errorOccurred(QStringLiteral("Modbus client not set"));
        return;
    }
    // использовать существующий ModbusClient API для отправки запроса
    _modbusClient->sendSingleRequest(mdu);
}

void ModbusClientService::writeRegister(const QString& registerName, const QVariant& value) {
    QModbusDataUnit mdu;
    if (!findMduByName(registerName, mdu)) {
        emit errorOccurred(QStringLiteral("Register mapping not found: %1").arg(registerName));
        return;
    }
    if (!_modbusClient) {
        emit errorOccurred(QStringLiteral("Modbus client not set"));
        return;
    }

    // Заполнение данных: mdu.values() — QVector<quint16>, нам нужно сконвертировать QVariant -> шестнадцатеричные слова
    QVector<quint16> vals;
    // Простейшая реализация: если value — числовой, запишем в первый word
    bool ok = false;
    qint64 ll = value.toLongLong(&ok);
    if (ok) {
        vals.append(static_cast<quint16>(ll & 0xFFFF));
    } else {
        double d = value.toDouble(&ok);
        if (ok) {
            // примитивная конвертация double -> two words (не оптимизировано)
            // Для корректной упаковки float/double нужен конкретный протокол/формат (ABCD/etc.)
            quint16 lo = static_cast<quint16>(static_cast<int>(d) & 0xFFFF);
            quint16 hi = static_cast<quint16>((static_cast<int>(d) >> 16) & 0xFFFF);
            vals.append(lo);
            vals.append(hi);
        } else {
            emit errorOccurred(QStringLiteral("Cannot convert QVariant to modbus words"));
            return;
        }
    }

    // сформируем QModbusDataUnit для записи (copy structure)
    QModbusDataUnit writeUnit(mdu.registerType(), mdu.startAddress(), static_cast<quint16>(vals.size()));
    writeUnit.setValue(0, vals.isEmpty() ? 0 : vals.at(0)); // minimal
    // В ModbusClient есть on_write(QModbusDataUnit), используем его
    _modbusClient->on_write(writeUnit);
}

void ModbusClientService::setDeviceUniqueId(const QString& uniqueId) {
    QMutexLocker locker(&_mutex);
    _deviceUniqueId = uniqueId;
}

QString ModbusClientService::deviceUniqueId() const {
    QMutexLocker locker(&_mutex);
    return _deviceUniqueId;
}

bool ModbusClientService::findMduByName(const QString& name, QModbusDataUnit& outMdu) const {
    if (!_modbusClient) return false;
    // ModbusBase содержит метод get_mdum() returning std::map<QString, QModbusDataUnit>
    auto mdum = _modbusClient->get_mdum(); // std::map<QString, QModbusDataUnit>
    auto it = mdum.find(name);
    if (it != mdum.end()) {
        outMdu = it->second;
        return true;
    }
    return false;
}

void ModbusClientService::onReadReady(const QModbusDataUnit& mdu) {
    // Обработка входящего mdu: пытаемся найти соответствующие именованные записи и эмитить значения
    handleIncomingMdu(mdu);
}

void ModbusClientService::handleIncomingMdu(const QModbusDataUnit& incoming) {
    if (!_modbusClient) return;

    auto mdum = _modbusClient->get_mdum();
    for (const auto &kv : mdum) {
        const QString regName = kv.first;
        const QModbusDataUnit& mapped = kv.second;

        // Сопоставляем тип таблицы и адреса (простая стратегия)
        if (mapped.registerType() == incoming.registerType() &&
            mapped.startAddress() == incoming.startAddress()) {

            // Получаем значения из incoming
            QVariantList values;
            const auto vals = incoming.values(); // QVector<quint16>
            for (auto v : vals) values << QVariant(static_cast<int>(v));

            // эмитим сигнал с именем регистра и списком значений
            emit registerValueRead(regName, values);
        }
    }
}

void ModbusClientService::onHostConnected() {
    emit deviceConnected(_deviceUniqueId);
}

void ModbusClientService::onHostDisconnected() {
    emit deviceDisconnected(_deviceUniqueId);
}

void ModbusClientService::onHostError(const QString& error) {
    emit errorOccurred(error);
}
