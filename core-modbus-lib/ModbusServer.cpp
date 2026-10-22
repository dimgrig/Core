#include "ModbusServer.h"
//#include "ui_modbus_server.h"

#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QUrl>

//#include "Config.h"

#include "Logger.h"

#include "Categories.h"
#include "JournalAccess.h"

#include "ModbusDataConverter.h"

//sudo setcap 'cap_net_bind_service=+ep' HIMIK-SERVER-TEST

ModbusServer::ModbusServer(QWidget *parent, QString config, ModbusSettings modbus_settings)
    : ModbusBase(parent, config, modbus_settings)
//    , ui(new Ui::fmodbus_server)
{
//    ui->setupUi(this);
    this->on_read_json(config);

    // this->ui->cb_connect_type->addItems(ConnectionType_sl);
    // this->ui->cb_baudrate->addItems(ModbusBaudRate_sl);
    // this->ui->cb_data_bits->addItems(ModbusDataBits_sl);
    // this->ui->cb_stop_bits->addItems(ModbusStopBits_sl);
    // this->ui->cb_parity->addItems(ModbusParity_sl);

    // connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->le_address, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->le_port, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);

    this->on_connect_type(static_cast<int>(ConnectionType_from_text(modbus_settings.MODBUS_TYPE.toStdString().c_str())));

    // _reconnectTimer = new QTimer(this);
    // _reconnectTimer->setSingleShot(true);
    // connect(_reconnectTimer, &QTimer::timeout, this, &ModbusServer::on_connect);

    _portCheckTimer = new QTimer(this);
    connect(_portCheckTimer, &QTimer::timeout, this, &ModbusServer::checkSerialPorts);

    // if (_connection_type == ConnectionTypeSerial) {
    //     _portCheckTimer->start(1000); // Проверка каждую секунду
    // }
}

ModbusServer::~ModbusServer() {
    disconnect(modbusDevice.get(), &QModbusServer::stateChanged, this, &ModbusServer::onModbusStateChanged);
    disconnect(modbusDevice.get(), &QModbusServer::stateChanged, this, &ModbusServer::on_state_changed);
    disconnect(modbusDevice.get(), &QModbusServer::errorOccurred, this, &ModbusServer::onModbusErrorOccurred);

    if (modbusDevice)
        modbusDevice->disconnectDevice();
}

void ModbusServer::updateDeviceMap()
{
    if (modbusDevice) {
        modbusDevice->setMap(this->get_reg(_modbus_settings.MODBUS_ADDRESS));
    }
}

void ModbusServer::updatePollingGroups() {

}

void ModbusServer::on_connect_type(int index) {
    _connection_type = static_cast<ConnectionType>(index);

    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        modbusDevice.reset();  // shared_ptr автоматически удалит объект
    }

    if (_connection_type == ConnectionTypeSerial) {
        modbusDevice = std::make_shared<QModbusRtuSerialSlave>();
    } else if (_connection_type == ConnectionTypeTCP) {
        modbusDevice = std::make_shared<QModbusTcpServer>();
    }

    // connect(modbusDevice, &QModbusDevice::errorOccurred, [this](QModbusDevice::Error) {
    //     qCDebug(logDebug()) << modbusDevice->errorString();
    //     QTimer::singleShot(1000, [=]() {
    //         this->on_connect();

    //         _request = false;
    //     });
    // });

    if (!modbusDevice) {
        if (_connection_type == ConnectionTypeSerial) {
            if (auto* j = JournalAccess::journal())
                j->warning(Category::Modbus, "Не удалось создать Modbus RTU слейв");
        } else if (_connection_type == ConnectionTypeTCP) {
            if (auto* j = JournalAccess::journal())
                j->warning(Category::Modbus, "Не удалось создать Modbus TCP сервер");
        }
    } else {
        connect(modbusDevice.get(), &QModbusServer::stateChanged, this, &ModbusServer::onModbusStateChanged);
        connect(modbusDevice.get(), &QModbusServer::stateChanged, this, &ModbusServer::on_state_changed);
        connect(modbusDevice.get(), &QModbusServer::errorOccurred, this, &ModbusServer::onModbusErrorOccurred);
        connect(modbusDevice.get(), &QModbusServer::dataWritten, this, &ModbusServer::updateRegisters);
    }
}

void ModbusServer::on_connect() {
    if (!modbusDevice)
        return;

    if (_connection_type == ConnectionTypeSerial) {
        QString portName = _modbus_settings.MODBUS_SERIAL_PORT;

        // Проверяем наличие порта в системе
        if (!isSerialPortAvailable(portName)) {
            QString errorMsg = tr("Serial port %1 not found in system").arg(portName);

            // Генерируем сигнал об ошибке
            emit host_error(errorMsg);

            // Планируем повторную проверку через некоторое время
            // _failedAttempts++;
            // scheduleReconnect();
            return;
        }
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

        modbusDevice->setServerAddress(_modbus_settings.MODBUS_ADDRESS);
//        modbusDevice->setTimeout(2000);
//        modbusDevice->setNumberOfRetries(3);
        if (!modbusDevice->connectDevice()) {
            qCDebug(logDebug) << tr("ModbusServer Connect failed: ") + modbusDevice->errorString();
        } else {
            if (_connection_type == ConnectionTypeSerial) {
                qCDebug(logDebug) << tr("ModbusServer Connected: ") + _modbus_settings.MODBUS_SERIAL_PORT << modbusDevice->state();
            } else if (_connection_type == ConnectionTypeTCP) {
                qCDebug(logDebug) << tr("ModbusServer Connected: ") + _modbus_settings.MODBUS_IP << modbusDevice->state();
            }
            this->setupDeviceData();
            this->updateDeviceMap();
            this->updatePollingGroups();
            for (const auto &[serverAddress, __mdum] : this->_mdum) {
                if (serverAddress == _modbus_settings.MODBUS_ADDRESS) {
                    for (const auto &value : __mdum) {
                        this->modbusDevice->setData(value.second);
                        //qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
                    }
                }
            }
        }
    } else {
        //modbusDevice->disconnectDevice();
    }
}

void ModbusServer::disconnectFromHost() {
    if (modbusDevice)
        modbusDevice->disconnectDevice();
}

void ModbusServer::onModbusStateChanged(int state) {
    QString state_str = ModbusSettingsConverter::modbus_state_to_str(modbusDevice->state());

    //this->ui->le_state->setText(state_str);
    //qCDebug(logDebug()) << modbusDevice->state();

    emit state_changed(state_str);

    if (state == QModbusDevice::UnconnectedState) {
        emit on_disconnected();
        emit host_disconnected();
        qCDebug(logDebug) << tr("ModbusServer Disconnect: ") << modbusDevice->state();
    } else if (state == QModbusDevice::ConnectedState) {
        emit on_connected();
        emit host_connected();
        if (_connection_type == ConnectionTypeSerial) {
            qCDebug(logDebug) << tr("ModbusServer Connected: ") + _modbus_settings.MODBUS_SERIAL_PORT << modbusDevice->state();
        } else if (_connection_type == ConnectionTypeTCP) {
            qCDebug(logDebug) << tr("ModbusServer Connected: ") + _modbus_settings.MODBUS_IP << modbusDevice->state();
        }
    }

    // if (state == QModbusDevice::ConnectedState) {
    //     _failedAttempts = 0;
    // }
}

void ModbusServer::onModbusErrorOccurred(QModbusDevice::Error error) {
    qCDebug(logDebug) << error << modbusDevice->errorString();

    // Для критических ошибок увеличиваем счетчик
    // if (error != QModbusDevice::NoError) {
    //     _failedAttempts++;
    //     scheduleReconnect();
    // }

    emit host_error(modbusDevice->errorString());
}

void ModbusServer::settings_change() {
    if (this->modbusDevice) {
        this->modbusDevice->disconnectDevice();
        this->on_connect();
    }
}

void ModbusServer::setRegister(int serverAddress, const QString &reg_name, double value) {
    auto regType = this->getRegisterType(serverAddress, reg_name);
    if (regType != QModbusDataUnit::Invalid) {
        this->setRegister(serverAddress, regType, reg_name, value);
    } else {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Modbus, QString("Не удалось определить тип регистра для \"%1\"").arg(reg_name));
    }
}

void ModbusServer::setRegisterString(int serverAddress, const QString &reg_name, const QString &value) {
    auto regType = this->getRegisterType(serverAddress, reg_name);
    if (regType != QModbusDataUnit::Invalid) {
        this->setRegisterString(serverAddress, regType, reg_name, value);
    } else {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Modbus, QString("Не удалось определить тип регистра для \"%1\"").arg(reg_name));
    }
}

void ModbusServer::setRegister(int serverAddress, QModbusDataUnit::RegisterType table, const QString &regName, double value) {
    if (!modbusDevice)
        return;

    if (_mdum.count(serverAddress) > 0) {
        if (_mdum.at(serverAddress).count(regName) > 0) {
            bool ok = true;
            Q_UNUSED(ok);

            int address = _mdum.at(serverAddress).at(regName).startAddress(); // + 1
            int size = _mdum.at(serverAddress).at(regName).valueCount();
            Q_UNUSED(size);

            //qCDebug(logDebug) << "setRegister" << "reg_name" << reg_name << "registerType" << table  << "value" << value << "startAddress" << address;

            switch (table) {
                case QModbusDataUnit::Coils:
                case QModbusDataUnit::DiscreteInputs:
                {
                    ok = modbusDevice->setData(table, address, value);
                    // Синхронизировать _mdum: без этого setupDeviceData()/setMap()
                    // при connectToHost() затирают значение нулями из _mdum.
                    QVector<quint16> values(1, static_cast<quint16>(value != 0 ? 1 : 0));
                    _mdum[serverAddress][regName].setValues(values);
                }
                break;
                case QModbusDataUnit::InputRegisters:
                case QModbusDataUnit::HoldingRegisters:
                {
                    auto reg_data_type_m = this->get_reg_data_type_m();
                    QString dataType = "double";
                    if (reg_data_type_m.count(serverAddress) > 0) {
                        if (reg_data_type_m.at(serverAddress).count(regName) > 0) {
                            dataType = reg_data_type_m.at(serverAddress).at(regName);
                        }
                    }

                    auto reg_scale_m = this->get_reg_scale_m();
                    double scale = 0.0;
                    if (reg_scale_m.count(serverAddress) > 0) {
                        if (reg_scale_m.at(serverAddress).count(regName) > 0) {
                            scale = reg_scale_m.at(serverAddress).at(regName);
                        }
                    }

                    QVector<quint16> values = ModbusDataConverter::variantToRegisters(value, regName, dataType, modbus_format());
                    _mdum[serverAddress][regName].setValues(values);
                    ok = modbusDevice->setData(_mdum[serverAddress][regName]);
                }
                break;
                default:

                break;
            }
            if (!ok) {
                if (auto* j = JournalAccess::journal())
                    j->warning(Category::Modbus, QString("Не удалось установить регистр: %1").arg(modbusDevice->errorString()));
            }
        }
    }

    // qCDebug(logDebug()) << _mdum.size();
    // for (const auto &value : _mdum) {
    //     qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
    // }
}

void ModbusServer::setRegisterString(int serverAddress, QModbusDataUnit::RegisterType table, const QString &regName, const QString &value) {
    if (!modbusDevice)
        return;

    if (_mdum.count(serverAddress) > 0) {
        if (_mdum.at(serverAddress).count(regName) > 0) {
            //qCDebug(logDebug) << "setRegisterString" << "reg_name" << reg_name << "registerType" << table << "value" << value << "startAddress" << address;

            bool ok = true;
            Q_UNUSED(ok);

            switch (table) {
            case QModbusDataUnit::InputRegisters:
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
                _mdum[serverAddress][regName].setValues(values);
                ok = modbusDevice->setData(_mdum[serverAddress][regName]);
                //qCDebug(logDebug) << "setRegisterString" << reg_name << value << "as" << values;
            }
            break;
            default:
                if (auto* j = JournalAccess::journal())
                    j->warning(Category::Modbus, QString("Запись строки не поддерживается для типа регистра %1").arg(table));
                break;
            }

            if (!ok) {
                if (auto* j = JournalAccess::journal())
                    j->warning(Category::Modbus, QString("Не удалось установить регистр: %1").arg(modbusDevice->errorString()));
            }
        }
    }
}

void ModbusServer::updateRegisters(QModbusDataUnit::RegisterType table, int address, int size) {
    //qCDebug(logDebug()) << "updateRegisters" << "registerType" << table << "startAddress" << address << "valueCount" << size;
    QModbusDataUnit mdu(table, address, size);

    auto mdum = this->update_mdu(_modbus_settings.MODBUS_ADDRESS, table, address, size, this->_mdum);

//    qCDebug(logDebug()) << "mdum" <<  mdum.size();
//    for (const auto &value : mdum) {
//        qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
//    }

    for (auto &[_serverAddress, _mdum] : mdum) {
        for (auto &mdu : _mdum) {
            quint16 value;
            switch (table) {
                case QModbusDataUnit::Coils:
                {
                    QVector<quint16> values;
                    for (unsigned int i = 0; i < mdu.second.valueCount(); ++i) {
                        modbusDevice->data(mdu.second.registerType(), quint16(mdu.second.startAddress() + i), &value);
                        values.push_back(value);
                    }
                    mdu.second.setValues(values);
                }
                break;
                case QModbusDataUnit::HoldingRegisters:
                {
                    QVector<quint16> values;
                    for (unsigned int i = 0; i < mdu.second.valueCount(); ++i) {
                        modbusDevice->data(mdu.second.registerType(), quint16(mdu.second.startAddress() + i), &value);
                        values.push_back(value);
                    }
                    mdu.second.setValues(values);
                }
                break;
            default:
                break;
            }

    //        qCDebug(logDebug()) << "updateRegisters" << "reg_name" << mdu.first << "registerType" << mdu.second.registerType() << "startAddress" << mdu.second.startAddress() <<
    //                               "valueCount" << mdu.second.valueCount() << "values" << mdu.second.values();
            emit read_ready(this->_modbus_settings.MODBUS_ADDRESS, mdu.first, mdu.second);
        }
    }
}
