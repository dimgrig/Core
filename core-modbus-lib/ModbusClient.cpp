#include "ModbusClient.h"
//#include "ui_modbus_client.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QUrl>

#include "Logger.h"

#include "ModbusDataConverter.h"

ModbusClient::ModbusClient(QWidget *parent, QString config, ModbusSettings modbus_settings)
    : ModbusBase(parent, config, modbus_settings)
//    , ui(new Ui::fmodbus_client)
{
//    ui->setupUi(this);

    // this->ui->cb_connect_type->addItems(ConnectionType_sl);
    // this->ui->cb_baudrate->addItems(ModbusBaudRate_sl);
    // this->ui->cb_data_bits->addItems(ModbusDataBits_sl);
    // this->ui->cb_stop_bits->addItems(ModbusStopBits_sl);
    // this->ui->cb_parity->addItems(ModbusParity_sl);

    // connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::on_connect_type);
    // connect(this->ui->le_address, &QLineEdit::editingFinished, this, &ModbusClient::settings_change);
    // connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &ModbusClient::settings_change);
    // connect(this->ui->le_port, &QLineEdit::editingFinished, this, &ModbusClient::settings_change);
    // connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &ModbusClient::settings_change);
    // connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::on_connect_type);
    // connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::on_connect_type);
    // connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::on_connect_type);
    // connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::on_connect_type);
    // connect(this->ui->sb_poll_timer, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value){
    //     emit poll_timer_changed(value);
    // });

    this->on_connect_type(static_cast<int>(ConnectionType_from_text(modbus_settings.MODBUS_TYPE.toStdString().c_str())));

    _reconnectTimer = new QTimer(this);
    _reconnectTimer->setSingleShot(true);
    connect(_reconnectTimer, &QTimer::timeout, this, &ModbusClient::on_connect);

    _portCheckTimer = new QTimer(this);
    connect(_portCheckTimer, &QTimer::timeout, this, &ModbusClient::checkSerialPorts);

    // if (_connection_type == ConnectionTypeSerial) {
    //     _portCheckTimer->start(1000); // Проверка каждую секунду
    // }
}

ModbusClient::~ModbusClient() {
    disconnect(modbusDevice.get(), &QModbusClient::stateChanged, this, &ModbusClient::onModbusStateChanged);
    disconnect(modbusDevice.get(), &QModbusClient::stateChanged, this, &ModbusClient::on_state_changed);
    disconnect(modbusDevice.get(), &QModbusClient::errorOccurred, this, &ModbusClient::onModbusErrorOccurred);

    if (_reconnectTimer) {
        _reconnectTimer->stop();
    }
    if (_portCheckTimer) {
        _portCheckTimer->stop();
    }

    _requestQueue.clear();
    _processingQueue = false;

    if (modbusDevice)
        modbusDevice->disconnectDevice();
}


bool ModbusClient::isServerResponsive(int serverAddress) const {
    bool res = false;
    if (_serverResponsiveState.count(serverAddress)) {
        res = _serverResponsiveState.at(serverAddress);
    }
    return res;
}

void ModbusClient::updateDeviceMap()
{
    if (modbusDevice) {
        //modbusDevice->setMap(this->_reg); //only for server
    }
}

void ModbusClient::updatePollingGroups() {
    _pollingGroups.clear();
    for (const auto& [serverAddress, __reg_v] : _reg_v) {
        for (const auto& groupMap : __reg_v) {
            for (const auto& unit : groupMap) {
                addPollingGroup(serverAddress, unit);
            }
        }
    }
    _currentPollGroupIndex = 0;
}

void ModbusClient::enqueueReadRequest(int serverAddress, const QModbusDataUnit& mdu, bool isPolling)
{
    if (_requestQueue.size() >= MAX_QUEUE_SIZE) {
        qCWarning(logWarning) << "Request queue full, dropping read request";
        return;
    }
    _requestQueue.enqueue({mdu, serverAddress, false, false, isPolling});
    if (!_requestInProgress) {
        processNextRequest();
    }
}
void ModbusClient::enqueueWriteRequest(int serverAddress, const QModbusDataUnit& mdu, bool highPriority)
{
    if (mdu.registerType() == QModbusDataUnit::DiscreteInputs ||
        mdu.registerType() == QModbusDataUnit::InputRegisters) {
        qCWarning(logWarning) << "Refusing to write to read-only register type:" << mdu.registerType();
        return;
    }

    if (_requestQueue.size() >= MAX_QUEUE_SIZE) {
        qCWarning(logWarning) << "Request queue full, dropping write request";
        return;
    }
    if (highPriority) {
        // Высокоприоритетные запросы в начало очереди
        _requestQueue.prepend({mdu, serverAddress, true, true, false});
        if (!_requestInProgress) {
            processNextRequest();
        }
    } else {
        _requestQueue.enqueue({mdu, serverAddress, true, false, false});
    }
}

void ModbusClient::startPolling()
{
    if (!_pollTimer) {
        _pollTimer = new QTimer(this);
        _pollTimer->setSingleShot(false);
        connect(_pollTimer, &QTimer::timeout, this, &ModbusClient::startPollingCycle);
    }
    if (_pollTimer && !_pollTimer->isActive()) {
        _pollTimer->start(this->_modbus_settings.MODBUS_POLL_TIMER);
    }
}

void ModbusClient::stopPolling()
{
    if (_pollTimer) {
        _pollTimer->stop();
    }
}

void ModbusClient::clearQueue()
{
    _requestQueue.clear();
    _requestInProgress = false;
    _pendingPollCount = 0;
    _pollingCycleActive = false;
}

void ModbusClient::on_connect_type(int index) {
    _connection_type = static_cast<ConnectionType>(index);

    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        modbusDevice.reset();  // shared_ptr автоматически удалит объект
    }

    if (_connection_type == ConnectionTypeSerial) {
        modbusDevice = std::make_shared<QModbusRtuSerialMaster>();
    } else if (_connection_type == ConnectionTypeTCP) {
        modbusDevice = std::make_shared<QModbusTcpClient>();
    }

    // connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
    //     qCDebug(logDebug()) << modbusDevice->errorString();
    //     QTimer::singleShot(1000, [=]() {
    //         this->on_connect();
    //     });
    // });

    if (!modbusDevice) {
        if (_connection_type == ConnectionTypeSerial) {
            qCDebug(logWarning) << (tr("Could not create Modbus master."));
        } else if (_connection_type == ConnectionTypeTCP) {
            qCDebug(logDebug) << (tr("Could not create Modbus client."));
        }
    } else {
        connect(modbusDevice.get(), &QModbusClient::stateChanged, this, &ModbusClient::onModbusStateChanged);
        connect(modbusDevice.get(), &QModbusClient::stateChanged, this, &ModbusClient::on_state_changed);
        connect(modbusDevice.get(), &QModbusClient::errorOccurred, this, &ModbusClient::onModbusErrorOccurred);
    }
}

void ModbusClient::on_connect() {
    if (!modbusDevice)
        return;

    if (!_requestQueue.isEmpty()) {
        qCDebug(logDebug) << "Clearing request queue on reconnect, size:" << _requestQueue.size();
        _requestQueue.clear();
    }
    _processingQueue = false;

    if (_connection_type == ConnectionTypeSerial) {
        QString portName = _modbus_settings.MODBUS_SERIAL_PORT;

        // Проверяем наличие порта в системе
        if (!isSerialPortAvailable(portName)) {
            QString errorMsg = tr("Serial port %1 not found in system").arg(portName);

            // Генерируем сигнал об ошибке
            emit host_error(errorMsg);

            // Планируем повторную проверку через некоторое время
            _failedAttempts++;
            scheduleReconnect();
            return;
        }

        _portCheckTimer->start(1000); // Проверка каждую секунду
    }

    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (_connection_type == ConnectionTypeSerial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, _modbus_settings.MODBUS_SERIAL_PORT);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, _modbus_settings.MODBUS_BAUDRATE); //QSerialPort::Baud19200
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, _modbus_settings.MODBUS_DATA_BITS); //QSerialPort::Data8
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, _modbus_settings.MODBUS_STOP_BITS); //QSerialPort::OneStop
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, _modbus_settings.MODBUS_PARITY); //QSerialPort::EvenParity
        } else if (_connection_type == ConnectionTypeTCP) {
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, _modbus_settings.MODBUS_IP);
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, _modbus_settings.MODBUS_PORT);
        }

        emit host_connecting();
        emit on_connecting();

        modbusDevice->setTimeout(1000);
        modbusDevice->setNumberOfRetries(0);
        if (!modbusDevice->connectDevice()) {
            qCDebug(logDebug) << tr("ModbusClient Connect failed: ") + modbusDevice->errorString();
        } else {
            if (_connection_type == ConnectionTypeSerial) {
                qCDebug(logDebug) << tr("ModbusClient Connected: ") + _modbus_settings.MODBUS_SERIAL_PORT << modbusDevice->state();
            } else if (_connection_type == ConnectionTypeTCP) {
                qCDebug(logDebug) << tr("ModbusClient Connected: ") + _modbus_settings.MODBUS_IP << modbusDevice->state();
            }
            this->setupDeviceData();
            this->updateDeviceMap();
            this->updatePollingGroups();
        }
    } else {
        //modbusDevice->disconnectDevice();
    }
}

void ModbusClient::disconnectFromHost() {
    if (modbusDevice)
        modbusDevice->disconnectDevice();

    this->_reconnectTimer->stop();
    this->_portCheckTimer->stop();
}

void ModbusClient::onModbusStateChanged(int state) {
    QString state_str = ModbusSettingsConverter::modbus_state_to_str(modbusDevice->state());

    //this->ui->le_state->setText(state_str);
    //qCDebug(logDebug()) << modbusDevice->state();

    emit state_changed(state_str);

    if (state == QModbusDevice::UnconnectedState) {
        emit on_disconnected();
        emit host_disconnected();
        qCDebug(logDebug) << tr("ModbusClient Disconnect: ") << modbusDevice->state();
    } else if (state == QModbusDevice::ConnectedState) {
        emit on_connected();
        emit host_connected();
        if (_connection_type == ConnectionTypeSerial) {
            qCDebug(logDebug) << tr("ModbusClient Connected: ") + _modbus_settings.MODBUS_SERIAL_PORT << modbusDevice->state();
        } else if (_connection_type == ConnectionTypeTCP) {
            qCDebug(logDebug) << tr("ModbusClient Connected: ") + _modbus_settings.MODBUS_IP << modbusDevice->state();
        }
    }

    if (state == QModbusDevice::ConnectedState) {
        _failedAttempts = 0;

        startPolling();
    } else if (state == QModbusDevice::UnconnectedState) {
        stopPolling();
        clearQueue();
    }

    if (state == QModbusDevice::ConnectedState) {
        // Сброс состояния для всех адресов, которые есть в _serverResponsiveState
        for (auto& [addr, responsive] : _serverResponsiveState) {
            if (!responsive) {
                responsive = true;
                emit serverHealthChanged(addr, true);
            }
            _serverRequestErrorCount[addr] = 0;
        }
    } else if (state == QModbusDevice::UnconnectedState) {
        for (auto& [addr, responsive] : _serverResponsiveState) {
            if (responsive) {
                responsive = false;
                emit serverHealthChanged(addr, false);
            }
        }
    }
}

void ModbusClient::onModbusErrorOccurred(QModbusDevice::Error error) {
    qCDebug(logDebug) << error << modbusDevice->errorString();

    // Для критических ошибок увеличиваем счетчик
    if (error != QModbusDevice::NoError) {
        _failedAttempts++;
        scheduleReconnect();
    }

    emit host_error(modbusDevice->errorString());
}

void ModbusClient::settings_change() {
    if (this->modbusDevice) {
        this->modbusDevice->disconnectDevice();
        this->on_connect();
    }
}

void ModbusClient::sendSingleRequest(int serverAddress, QModbusDataUnit mdu) {
    if (auto *reply = modbusDevice->sendReadRequest(mdu, serverAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, &ModbusClient::on_read_ready);
        } else {
            _processingQueue = false;
            processNextRequest();
            delete reply;
        }
    } else {
        _processingQueue = false;
        _failedAttempts++;
        scheduleReconnect();
        processNextRequest();
    }
}

void ModbusClient::on_read(int serverAddress, QModbusDataUnit mdu) {
    enqueueReadRequest(serverAddress, mdu);
}

void ModbusClient::on_read(int serverAddress, QModbusDataUnit::RegisterType type, int startAddress, quint16 numberOfEntries) {
    //this->on_read(QModbusDataUnit(type, startAddress, numberOfEntries));
    enqueueReadRequest(serverAddress, QModbusDataUnit(type, startAddress, numberOfEntries));
}

void ModbusClient::on_read_ready() {
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) {
        _processingQueue = false;
        processNextRequest();
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        emit read_ready(reply->serverAddress(), unit);
        _failedAttempts = 0;
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qCDebug(logDebug) << (tr("Read response error: %1 (Modbus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16));
    } else {
        qCDebug(logDebug) << (tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16));
    }

    reply->deleteLater();
    _processingQueue = false;
    processNextRequest();
}

void ModbusClient::on_write(int serverAddress, QModbusDataUnit mdu) {
    enqueueWriteRequest(serverAddress, mdu, true);
}

void ModbusClient::on_write(int serverAddress, QModbusDataUnit::RegisterType type, int startAddress, quint16 numberOfEntries) {
    //this->on_write(QModbusDataUnit(type, startAddress, numberOfEntries));
    enqueueWriteRequest(serverAddress, QModbusDataUnit(type, startAddress, numberOfEntries), true);
}

void ModbusClient::setRegister(int serverAddress, const QString &reg_name, double value) {
    auto regType = this->getRegisterType(serverAddress, reg_name);
    if (regType != QModbusDataUnit::Invalid) {
        this->setRegister(serverAddress, regType, reg_name, value);
    } else {
        qCDebug(logWarning) << "Unable to get register type from name" << reg_name;
    }
}

void ModbusClient::setRegisterString(int serverAddress, const QString &reg_name, const QString &value) {
    auto regType = this->getRegisterType(serverAddress, reg_name);
    if (regType != QModbusDataUnit::Invalid) {
        this->setRegisterString(serverAddress, regType, reg_name, value);
    } else {
        qCDebug(logWarning) << "Unable to get register type from name" << reg_name;
    }
}

void ModbusClient::setRegister(int serverAddress, QModbusDataUnit::RegisterType table, const QString &regName, double value) {
    if (!modbusDevice)
        return;

    if (_mdum.count(serverAddress) > 0) {
        if (_mdum.at(serverAddress).count(regName) > 0) {
            bool ok = true;
            Q_UNUSED(ok);

            QModbusDataUnit mdu = _mdum.at(serverAddress).at(regName);

            switch (table) {
                break;
                case QModbusDataUnit::Coils:
                {
                    QVector<quint16> values {(quint16)value};
                    mdu.setValues(values);
                    this->on_write(serverAddress, mdu);
                    //qCDebug(logDebug) << "setRegister" << reg_name << value;
                }
                break;
                case QModbusDataUnit::HoldingRegisters:
                {
                    auto reg_data_type_m = this->get_reg_data_type_m();
                    QString dataType = "double";
                    if (reg_data_type_m.count(serverAddress) > 0) {
                        if (reg_data_type_m.at(serverAddress).count(regName) > 0) {
                            dataType = reg_data_type_m.at(serverAddress).at(regName);
                        }
                    }

                    QVector<quint16> values = ModbusDataConverter::variantToRegisters(value, regName, dataType, modbus_format());
                    mdu.setValues(values);
                    this->on_write(serverAddress, mdu);

                    //qCDebug(logDebug) << "setRegister" << reg_name << value << mdu.values();
                }
                break;
                default:

                break;
            }
    //        if (!ok)
    //            statusBar()->showMessage(tr("Could not set register: ") + modbusDevice->errorString());
        }
    }

    // qCDebug(logDebug()) << _mdum.size();
    // for (const auto &value : _mdum) {
    //     qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
    // }
}

void ModbusClient::setRegisterString(int serverAddress, QModbusDataUnit::RegisterType table, const QString &regName, const QString &value) {
    if (!modbusDevice)
        return;

    if (_mdum.count(serverAddress) > 0) {
        if (_mdum.at(serverAddress).count(regName) > 0) {
            QModbusDataUnit mdu = _mdum.at(serverAddress).at(regName);
            int size = mdu.valueCount();
            int maxBytes = size * 2;
            Q_UNUSED(maxBytes);

            switch (table) {
            case QModbusDataUnit::HoldingRegisters:
            {
                auto reg_data_type_m = this->get_reg_data_type_m();
                QString dataType = "double";
                if (reg_data_type_m.count(serverAddress) > 0) {
                    if (reg_data_type_m.at(serverAddress).count(regName) > 0) {
                        dataType = reg_data_type_m.at(serverAddress).at(regName);
                    }
                }
                QVector<quint16> values = ModbusDataConverter::variantToRegisters(value, regName, dataType, modbus_format());
                mdu.setValues(values);
                this->on_write(serverAddress, mdu);
                //qCDebug(logDebug) << "setRegisterString" << reg_name << value << "as" << values;
            }
            break;
            default:
                qCDebug(logWarning) << "String write not supported for register type:" << table;
                break;
            }
        }
    }
}

void ModbusClient::scheduleReconnect() {
    if (_connection_type == ConnectionTypeSerial) {
        QString portName = _modbus_settings.MODBUS_SERIAL_PORT;

        if (!isSerialPortAvailable(portName)) {
            // Если порта нет, увеличиваем интервал переподключения
            int delay = 5000; // 5 секунд для Serial, когда порта нет
            qCDebug(logWarning) << "Serial port" << portName << "not available, retrying in" << delay << "ms";
            _reconnectTimer->start(delay);
            return;
        }
    }

    if (_failedAttempts >= MAX_FAILED_ATTEMPTS) {
        qCDebug(logDebug) << "Max failed attempts reached, waiting before reconnect";
        _reconnectTimer->start(5000);
    } else {
        // Исправленный exponential backoff: 1s, 2s, 4s...
        int delay = 1000 * (1 << (_failedAttempts - 1));
        delay = qMin(delay, 5000); // Максимум 5 секунд
        qCDebug(logDebug) << "Scheduling reconnect in" << delay << "ms, attempt" << _failedAttempts;
        _reconnectTimer->start(delay);
    }

    // Очищаем очередь при планировании переподключения
    if (!_requestQueue.isEmpty()) {
        qCDebug(logDebug) << "Clearing queue due to reconnect, had" << _requestQueue.size() << "pending requests";
        _requestQueue.clear();
    }
    _processingQueue = false;
}

void ModbusClient::processNextRequest()
{
    if (_requestInProgress || _requestQueue.isEmpty()) return;
    if (!modbusDevice || (modbusDevice->state() != QModbusDevice::ConnectedState)) {
        clearQueue();
        return;
    }

    Request req = _requestQueue.dequeue();
    _requestInProgress = true;

    // qCDebug(logDebug) << "Sending request: addr=" << req.serverAddress
    //                   << "type=" << req.unit.registerType()
    //                   << "start=" << req.unit.startAddress()
    //                   << "count=" << req.unit.valueCount()
    //                   << (req.isWrite ? "WRITE" : "READ");

    QModbusReply *reply = nullptr;
    if (req.isWrite) {
        reply = modbusDevice->sendWriteRequest(req.unit, req.serverAddress);
        _lastReadUnit = req.unit;
    } else {
        reply = modbusDevice->sendReadRequest(req.unit, req.serverAddress);
        _lastWriteUnit = req.unit;
    }

    if (reply) {
        connect(reply, &QModbusReply::finished, this, [this, reply, req]() {
            bool success = (reply->error() == QModbusDevice::NoError);
                int addr = req.serverAddress;

                if (success) {
                    // Успешный ответ – сбрасываем счётчик ошибок для этого адреса
                    _serverRequestErrorCount[addr] = 0;
                    if (!_serverResponsiveState[addr]) {
                        _serverResponsiveState[addr] = true;
                        emit serverHealthChanged(addr, true);
                    }
                } else {
                    // Ошибка – увеличиваем счётчик
                    _serverRequestErrorCount[addr]++;
                    if (_serverRequestErrorCount[addr] >= _requestErrorThreshold) {
                        if (_serverResponsiveState[addr]) {
                            _serverResponsiveState[addr] = false;
                            emit serverHealthChanged(addr, false);
                        }
                    }
                }

                if (!req.isWrite && success) {
                    emit read_ready(req.serverAddress, reply->result());
                }

                reply->deleteLater();
                _requestInProgress = false;

                if (req.isPolling) {
                    _pendingPollCount--;
                    if (_pendingPollCount == 0) _pollingCycleActive = false;
                }

                processNextRequest();
        });
    } else {
        //qCWarning(logWarning) << "Failed to send request:" << modbusDevice->errorString();
        _requestInProgress = false;
        int addr = req.serverAddress;
        _serverRequestErrorCount[addr]++;
        if (_serverRequestErrorCount[addr] >= _requestErrorThreshold) {
            if (_serverResponsiveState[addr]) {
                _serverResponsiveState[addr] = false;
                emit serverHealthChanged(addr, false);
            }
        }
        if (req.isPolling) {
            _pendingPollCount--;
            if (_pendingPollCount == 0) {
                _pollingCycleActive = false;
            }
        }
        processNextRequest();
    }
}

void ModbusClient::startPollingCycle()
{
    if (_pollingGroups.empty() || _pollingCycleActive) return;
    if (modbusDevice->state() != QModbusDevice::ConnectedState) return;

    _pollingCycleActive = true;
    _pendingPollCount = _pollingGroups.size();
    for (const auto& group : _pollingGroups) {
        enqueueReadRequest(group.serverAddress, group.unit, true); // помечаем как опросный
    }
}

void ModbusClient::addPollingGroup(int serverAddress, const QModbusDataUnit& unit) {
    // Проверяем, нет ли уже такой же группы (опционально)
    for (const auto& group : _pollingGroups) {
        if (group.unit.registerType() == unit.registerType() &&
            group.unit.startAddress() == unit.startAddress() &&
            group.unit.valueCount() == unit.valueCount() &&
            group.serverAddress == serverAddress) {
            // Уже есть, не добавляем повторно
            return;
        }
    }
    _pollingGroups.push_back({serverAddress, unit});
    // qCDebug(logDebug()) << "Added polling group for address" << serverAddress
    //                     << "type" << unit.registerType()
    //                     << "start" << unit.startAddress()
    //                     << "count" << unit.valueCount();
}

void ModbusClient::removePollingGroupsForAddress(int serverAddress) {
    auto it = std::remove_if(_pollingGroups.begin(), _pollingGroups.end(),
                             [serverAddress](const PollingGroup& group) {
                                 return group.serverAddress == serverAddress;
                             });
    if (it != _pollingGroups.end()) {
        size_t removed = std::distance(it, _pollingGroups.end());
        _pollingGroups.erase(it, _pollingGroups.end());
        //qCDebug(logDebug()) << "Removed" << removed << "polling groups for address" << serverAddress;

        // Корректируем индекс, если нужно
        if (_currentPollGroupIndex >= (int)_pollingGroups.size()) {
            _currentPollGroupIndex = 0;
        }
    }
}

void ModbusClient::clearPollingGroups() {
    _pollingGroups.clear();
    _currentPollGroupIndex = 0;
    //qCDebug(logDebug()) << "Cleared all polling groups";
}
