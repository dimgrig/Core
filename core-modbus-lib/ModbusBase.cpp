#include "ModbusBase.h"

#include <QModbusDevice>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QScopedValueRollback>

#include "Logger.h"
#include "ModbusRegisterSerializer.h"

void mdum_v_to_reg_v(const std::vector<std::pair<QString, QModbusDataUnit>> &mdum_v, std::vector<QModbusDataUnitMap> &reg_v) {
    for (const auto &_mdu : mdum_v) {
        if (reg_v.empty()) {
            QModbusDataUnitMap mdum;
            mdum[_mdu.second.registerType()] = _mdu.second;
            reg_v.push_back(mdum);
        } else {
            //qCDebug(logDebug()) << reg_v.back()[_mdu.second.registerType()].startAddress() + reg_v.back()[_mdu.second.registerType()].valueCount() << _mdu.second.startAddress();
            if (static_cast<unsigned int>((reg_v.back()[_mdu.second.registerType()].startAddress()) + reg_v.back()[_mdu.second.registerType()].valueCount()) ==
                static_cast<unsigned int>((_mdu.second.startAddress()))) {
                //reg_v.back()[_mdu.second.registerType()].setValueCount(reg_v.back()[_mdu.second.registerType()].valueCount() + _mdu.second.valueCount());
                auto values = reg_v.back()[_mdu.second.registerType()].values();
                for (unsigned int i = 0; i < _mdu.second.valueCount(); ++i) {
                    values.push_back(0);
                }
                reg_v.back()[_mdu.second.registerType()].setValues(values);
//                qCDebug(logDebug()) << reg_v.back()[_mdu.second.registerType()].registerType() << reg_v.back()[_mdu.second.registerType()].startAddress() <<
//                                       reg_v.back()[_mdu.second.registerType()].valueCount() << reg_v.back()[_mdu.second.registerType()].values();
            } else {
                QModbusDataUnitMap mdum;
                mdum[_mdu.second.registerType()] = _mdu.second;
                reg_v.push_back(mdum);
            }
        }
    }
}

ModbusBase::ModbusBase(QWidget *parent, QString config, ModbusSettings modbus_settings) :
    QWidget(parent) ,
    _modbus_settings(modbus_settings)
{
    this->on_read_json(config);
}

bool ModbusBase::isConnected() const {
    if (this->device()) {
        return (this->device()->state() == QModbusDevice::ConnectedState);
    } else {
        return false;
    }
}

QModbusDataUnit::RegisterType ModbusBase::getRegisterType(int serverAddress, const QString regName) const {
    QModbusDataUnit::RegisterType res = QModbusDataUnit::Invalid;
    if (_mdum.count(serverAddress) > 0) {
        if (_mdum.at(serverAddress).count(regName) > 0) {
            QModbusDataUnit mdu = this->_mdum.at(serverAddress).at(regName);
            res = mdu.registerType();
        }
    }
    return res;
}

void ModbusBase::set_modbus_settings(ModbusSettings modbus_settings) {
    _modbus_settings = modbus_settings;

    this->on_connect_type(static_cast<int>(ConnectionType_from_text(modbus_settings.MODBUS_TYPE.toStdString().c_str())));

    // if (_connection_type == ConnectionTypeSerial) {
    //     _portCheckTimer->start(1000); // Проверка каждую секунду
    // }
};

void ModbusBase::set_modbus_config(QString config) {
    this->on_read_json(config);
}

QModbusDataUnitMap ModbusBase::get_reg(int serverAddress) const {
    if (this->_reg.count(serverAddress)) {
        return this->_reg.at(serverAddress);
    }
    return QModbusDataUnitMap();
}
std::vector<QModbusDataUnitMap> ModbusBase::get_reg_v(int serverAddress) const {
    if (this->_reg_v.count(serverAddress)) {
        return this->_reg_v.at(serverAddress);
    }
    return {};
}

void ModbusBase::setupDeviceData() {

    for (const auto &[serverAddress, mdum] : this->_mdum) {
        QModbusDataUnitMap reg;
        for (const auto &value : mdum) {
            if (reg.count(value.second.registerType()) == 0) {
                reg.insert(value.second.registerType(), value.second);
                reg[value.second.registerType()].setStartAddress(reg[value.second.registerType()].startAddress()); // + 1
            } else {
                if (reg[value.second.registerType()].startAddress() > value.second.startAddress()) {
                    reg[value.second.registerType()].setValueCount(reg[value.second.registerType()].startAddress() -
                                                                   value.second.startAddress() +
                                                                   reg[value.second.registerType()].valueCount());
                    reg[value.second.registerType()].setStartAddress(value.second.startAddress()); // + 1
                } else {
                    if ((reg[value.second.registerType()].startAddress() +
                         reg[value.second.registerType()].valueCount()) >=
                        (value.second.startAddress() + value.second.valueCount())) {
                        ; //reg[value.second.registerType()].setValueCount(value.second.startAddress());
                    } else {
                        reg[value.second.registerType()].setValueCount(value.second.startAddress() -
                                                                       reg[value.second.registerType()].startAddress() +
                                                                       value.second.valueCount());
                    }
                }
            }
        }

        {
            //size fix
            for (const auto &r : reg) {
                QModbusDataUnit mdu = QModbusDataUnit(r.registerType(), r.startAddress(), r.valueCount());
                reg.insert(r.registerType(), mdu);
            }
        }

        //manually - not calculated from json
        //reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 28 });

        //    qCDebug(logDebug()) << "QModbusDataUnitMap" << reg.size();
        //    for (const auto &r : reg) {
        //        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
        //    }

        this->_reg[serverAddress] = reg;
    }

    ////*************************
    auto mdum_m = this->get_sort_by_start_address_mdum_v();
    for (const auto &[serverAddress, mdum_v] : mdum_m) {
        std::vector<QModbusDataUnitMap> reg_di_v;
        std::vector<QModbusDataUnitMap> reg_coil_v;
        std::vector<QModbusDataUnitMap> reg_ir_v;
        std::vector<QModbusDataUnitMap> reg_hr_v;

        std::vector<std::pair<QString, QModbusDataUnit>> mdum_di_v;
        std::vector<std::pair<QString, QModbusDataUnit>> mdum_coil_v;
        std::vector<std::pair<QString, QModbusDataUnit>> mdum_ir_v;
        std::vector<std::pair<QString, QModbusDataUnit>> mdum_hr_v;

        for (const auto &_mdu : mdum_v) {
            auto mdu = _mdu.second;
            switch (mdu.registerType()) {
                case QModbusDataUnit::DiscreteInputs:
                    mdum_di_v.push_back(_mdu);
                break;
                case QModbusDataUnit::Coils:
                    mdum_coil_v.push_back(_mdu);
                break;
                case QModbusDataUnit::InputRegisters:
                    mdum_ir_v.push_back(_mdu);
                break;
                case QModbusDataUnit::HoldingRegisters:
                    mdum_hr_v.push_back(_mdu);
                break;
                default:

                break;
            }
        }

    //    qCDebug(logDebug()) << "mdum_di_v";
    //    for (const auto &mdum : mdum_di_v) {
    //        auto r = mdum.second;
    //        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
    //    }
    //    qCDebug(logDebug()) << "mdum_coil_v";
    //    for (const auto &mdum : mdum_coil_v) {
    //        auto r = mdum.second;
    //        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
    //    }
    //    qCDebug(logDebug()) << "mdum_ir_v";
    //    for (const auto &mdum : mdum_ir_v) {
    //        auto r = mdum.second;
    //        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
    //    }
    //    qCDebug(logDebug()) << "mdum_hr_v";
    //    for (const auto &mdum : mdum_hr_v) {
    //        auto r = mdum.second;
    //        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
    //    }

        mdum_v_to_reg_v(mdum_di_v, reg_di_v);
        mdum_v_to_reg_v(mdum_coil_v, reg_coil_v);
        mdum_v_to_reg_v(mdum_ir_v, reg_ir_v);
        mdum_v_to_reg_v(mdum_hr_v, reg_hr_v);

        _reg_v[serverAddress].clear();
        _reg_v[serverAddress].insert(_reg_v[serverAddress].end(), reg_di_v.begin(), reg_di_v.end());
        _reg_v[serverAddress].insert(_reg_v[serverAddress].end(), reg_coil_v.begin(), reg_coil_v.end());
        _reg_v[serverAddress].insert(_reg_v[serverAddress].end(), reg_ir_v.begin(), reg_ir_v.end());
        _reg_v[serverAddress].insert(_reg_v[serverAddress].end(), reg_hr_v.begin(), reg_hr_v.end());

    //    qCDebug(logDebug()) << "std::vector<QModbusDataUnitMap>" << _reg_v.size() << reg_di_v.size() << reg_coil_v.size() << reg_ir_v.size() << reg_hr_v.size();
    //    for (const auto &reg : _reg_v) {
    //        for (const auto &r : reg) {
    //            qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
    //        }
    //    }
    }
}

std::map<int, std::map<QString, QModbusDataUnit>> ModbusBase::update_mdu(int serverAddress,
                                                          QModbusDataUnit::RegisterType table,
                                                          unsigned int address,
                                                          unsigned int size,
                                                          std::map<int, std::map<QString, QModbusDataUnit>> &mdum) {
    std::map<int, std::map<QString, QModbusDataUnit>> res;
    for (const auto &[_serverAddress, _mdum] : mdum) {
        if (_serverAddress == serverAddress) {
            for (const auto &mdu : _mdum) {
                if (mdu.second.registerType() == table) {
                    if ((static_cast<unsigned int>(mdu.second.startAddress()) <= address) &&
                        ((static_cast<unsigned int>(mdu.second.startAddress()) + mdu.second.valueCount()) > address)) {
                        res[_serverAddress][mdu.first] = mdu.second;
                        break;
                    }
                    if ((static_cast<unsigned int>(mdu.second.startAddress()) >= address) &&
                        (static_cast<unsigned int>(mdu.second.startAddress()) < (address + size))) {
                        res[_serverAddress][mdu.first] = mdu.second;
                    }
                }
            }
        }
    }
    return res;
}

bool ModbusBase::on_write_json(const QString& config) {
    QString filename = config.isEmpty() ? _config : config;
    if (filename.isEmpty()) {
        return false;
    }

    QString json = ModbusRegisterSerializer::serialize(this->getServerAddress(), this->_mdum, this->_reg_data_type_m, this->_transactionalGroups);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(json.toStdString().c_str());
    file.close();
    return true;
}

void ModbusBase::on_read_json(QString config) {
    QString filename;
    if (config == "") {
        filename = ":json/json/modbus.json";
        //check file in app dir
        QFileInfo check_file("modbus.json");
        if (check_file.exists() && check_file.isFile()) {
            filename = "modbus.json";
        } else {
            //else use from resources
        }
    } else {
        filename = config;
    }

    _config = filename;
    emit config_changed(filename);

    //qCDebug(logDebug()) << "Modbus table config file" << filename;

    QScopedPointer<QFile> mapFile(new QFile(filename));
    mapFile.data()->open(QIODevice::ReadOnly);
    QString json = mapFile.data()->readAll();
    mapFile.data()->close();

    auto [mdum, reg_data_type_m, transactionalGroups] = ModbusRegisterSerializer::deserialize(this->getServerAddress(), json);
    this->_mdum.clear();
    this->_reg_data_type_m.clear();
    this->_transactionalGroups.clear();
    this->_mdum = mdum;
    this->_reg_data_type_m = reg_data_type_m;
    this->_transactionalGroups = transactionalGroups;

    //filter complex regs
//    auto _mdum_copy = _mdum;
//    _mdum.clear();
//    for (auto &value : _mdum_copy) {
//        if ((value.first.end() - 1)->isDigit()) {
//            QStringList sl = value.first.split("_");
//            sl.pop_back();
//            QString reg_name = sl.join("_");
//            if (_mdum.count(reg_name) > 0) {
//                QModbusDataUnit mdu = _mdum.at(reg_name);
//                mdu.setValueCount(mdu.valueCount() + value.second.valueCount());
//                _mdum[reg_name] = mdu;
//            } else {
//                _mdum[reg_name] = value.second;
//            }
//        } else {
//            _mdum[value.first] = value.second;
//        }
//    }

    // qCDebug(logDebug()) << "--------- mdum" <<  this->_mdum.size() << "------------";
    // for (const auto &value : this->_mdum) {
    //     QString reg_type;
    //     if (_reg_type_m.count(value.first))  {
    //         reg_type = _reg_type_m[value.first];
    //     }
    //     qCDebug(logDebug()) << "reg_name" << value.first << "startAddress" << value.second.startAddress() <<
    //                            "valueCount" << value.second.valueCount() << "values" << value.second.values() << "reg_type" << reg_type;
    // }
}

std::map<int, std::vector<std::pair<QString, QModbusDataUnit>>> ModbusBase::get_sort_by_start_address_mdum_v() const {
    std::map<int, std::vector<std::pair<QString, QModbusDataUnit>>> res;
    for (const auto &[serverAddress, _mdu] : this->_mdum) {
        std::vector<std::pair<QString, QModbusDataUnit>> mdum_v;
        for (const auto &__mdu : _mdu) {
            mdum_v.push_back({__mdu.first, __mdu.second});
        }
        std::sort(mdum_v.begin(), mdum_v.end(), [](const std::pair<QString, QModbusDataUnit> &l, const std::pair<QString, QModbusDataUnit> &r){
            return l.second.startAddress() < r.second.startAddress();
        });
        //    for (const auto &_mdu : mdum_v) {
        //        qCDebug(logDebug()) << _mdu.first;
        //    }
        res[serverAddress] = mdum_v;
    }

    return res;
}

void ModbusBase::set_modbus_params(int serverAddress,
                                   std::map<int, std::map<QString, QModbusDataUnit>> mdum,
                                   std::map<int, std::map<QString, QString>> reg_data_type_m,
                                   std::map<int, std::map<QString, QStringList>> transactionalGroups) {
    this->_mdum[serverAddress] = mdum[serverAddress];
    this->_reg_data_type_m[serverAddress] = reg_data_type_m[serverAddress];
    this->_transactionalGroups[serverAddress] = transactionalGroups[serverAddress];

    this->setupDeviceData();
    this->updateDeviceMap();
    this->updatePollingGroups();
}

void ModbusBase::add_modbus_params(int serverAddress, QString reg_name, QModbusDataUnit mdu, QString reg_data_type) {
    this->_mdum[serverAddress][reg_name] = mdu;
    this->_reg_data_type_m[serverAddress][reg_name] = reg_data_type;

    this->setupDeviceData();
    this->updateDeviceMap();
    this->updatePollingGroups();
}

bool ModbusBase::isSerialPortAvailable(const QString &portName) {
    if (portName.isEmpty()) {
        return false;
    }

#ifdef Q_OS_WIN
    // Получаем список доступных портов
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // Ищем нужный порт
    for (const QSerialPortInfo &port : ports) {
        if (port.portName() == portName ||
            port.systemLocation() == portName) {
            //qCDebug(logDebug) << "Serial port found:" << port.portName();
            return true;
        }
    }

    // qCDebug(logDebug) << "Serial port not found:" << portName
    //                     << "Available ports:";
    // for (const QSerialPortInfo &port : ports) {
    //     qCDebug(logDebug) << "  -" << port.portName()
    //     << "(" << port.description() << ")";
    // }

    return false;
#else
    const QString path = portName.startsWith('/')
                             ? portName
                             : QStringLiteral("/dev/") + portName;

    return QFile::exists(path);
#endif
}


void ModbusBase::checkSerialPorts() {
    if (_connection_type != ConnectionTypeSerial) {
        _portCheckTimer->stop();
        return;
    }

    // Если предыдущая проверка ещё идёт — не начинаем новую
    static bool inProgress = false;
    if (inProgress) return;
    QScopedValueRollback<bool> guard(inProgress, true);

    QString portName = _modbus_settings.MODBUS_SERIAL_PORT;
    bool isAvailable = isSerialPortAvailable(portName);

    // Если порт стал доступен и мы не подключены - пытаемся подключиться
    if (isAvailable && this->state() != QModbusDevice::ConnectedState) {
        //qCDebug(logInfo) << "Serial port" << portName << "became available, attempting to connect";
        on_connect();
    }
    // Если порт исчез и мы подключены - отключаемся
    else if (!isAvailable && this->state() == QModbusDevice::ConnectedState) {
        //qCDebug(logDebug) << "Serial port" << portName << "disappeared, disconnecting";
        this->device()->disconnectDevice();
        emit host_error(tr("Serial port %1 disconnected").arg(portName));
    }
}

