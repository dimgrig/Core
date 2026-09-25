#include "BindingManager.h"

#include <qglobal.h>

#include "Logger.h"
#include "ModbusClient.h"
#include "ModbusDataConverter.h"

#include "CheckBoxBinding.h"
#include "DoubleSpinBoxBinding.h"
#include "LabelBinding.h"
#include "LineEditBinding.h"
#include "SpinBoxBinding.h"

#include "Categories.h"
#include "JournalAccess.h"

BindingManager::BindingManager(int serverAddress,
                               std::shared_ptr<ModbusClient> modbusClient,
                               QObject* parent)
    : QObject(parent)
    , m_serverAddress(serverAddress)
    , m_modbusClient(modbusClient) {
    loadTransactionalGroups();
}

BindingManager::~BindingManager() {   
    clearBindings();

    if (m_modbusClient) {
        disconnect(m_modbusClient.get(), nullptr, this, nullptr);
        disconnect(this, nullptr, m_modbusClient.get(), nullptr);

        m_modbusClient->disconnectFromHost();
    }
}

void BindingManager::addBinding(std::shared_ptr<RegisterBinding> binding) {
    if (!binding || !binding->isValid()) return;

    QString regName = binding->registerName();
    m_bindings[regName].append(binding);

    connect(binding.get(), &RegisterBinding::valueChanged,
            this, &BindingManager::onBindingValueChanged);
    connect(binding.get(), &RegisterBinding::registerValueRequested,
            this, &BindingManager::onRegisterValueRequested);
    connect(binding.get(), &RegisterBinding::readRequested,
            this, &BindingManager::onReadRequested);

    // Если значение уже в кэше, обновляем UI
    if (m_registerCache.contains(regName)) {
        binding->updateFromRegister(m_registerCache[regName]);
    }
}

void BindingManager::removeBinding(const QString& regName) {
    if (m_bindings.contains(regName)) {
        for (auto& binding : m_bindings[regName]) {
            disconnect(binding.get(), nullptr, this, nullptr);
        }
        m_bindings.remove(regName);
    }
}

void BindingManager::removeBinding(std::shared_ptr<RegisterBinding> binding) {
    if (!binding) return;

    QString regName = binding->registerName();
    if (m_bindings.contains(regName)) {
        m_bindings[regName].removeAll(binding);
        if (m_bindings[regName].isEmpty()) {
            m_bindings.remove(regName);
        }
        disconnect(binding.get(), nullptr, this, nullptr);
    }
}

void BindingManager::clearBindings() {
    for (auto& bindings : m_bindings) {
        for (auto& binding : bindings) {
            disconnect(binding.get(), nullptr, this, nullptr);
        }
    }
    m_bindings.clear();
    m_registerCache.clear();
    m_updatingBindings.clear();
}

QList<std::shared_ptr<RegisterBinding>> BindingManager::get_bindings(const QString& regName) const {
    if (m_bindings.contains(regName)) {
        return m_bindings[regName];
    } else {
        return {};
    }
}

void BindingManager::processModbusData(int serverAddress, const QModbusDataUnit& unit) {
    if (serverAddress != m_serverAddress) return;

    auto mdum = m_modbusClient->get_mdum();

    for (const auto& [_serverAddress, _mdum] : mdum) {
        if (_serverAddress == serverAddress) {
            for (const auto& regPair : _mdum) {
                const QString& regName = regPair.first;
                const QModbusDataUnit& regUnit = regPair.second;

                if (regUnit.registerType() == unit.registerType()) {
                    unsigned int start = regUnit.startAddress();
                    unsigned int count = regUnit.valueCount();

                    if ((start + count) > static_cast<unsigned int>(unit.startAddress()) &&
                        start < (static_cast<unsigned int>(unit.startAddress()) + unit.valueCount())) {

                        unsigned int offset = qMax((unsigned int)0, start - static_cast<unsigned int>(unit.startAddress()));
                        unsigned int copyCount = qMin(count, (unsigned int)(unit.valueCount() - offset));

                        if (copyCount > 0) {
                            QVector<quint16> data(copyCount);
                            for (unsigned int i = 0; i < copyCount; ++i) {
                                data[i] = unit.values().at(offset + i);
                            }

                            // Обновляем кэш
                            updateRegisterCache(regName, data);

                            // Обновляем все привязанные элементы UI
                            if (m_bindings.contains(regName)) {
                                for (auto& binding : m_bindings[regName]) {
                                    if (!binding) {
                                        if (auto* j = JournalAccess::journal())
                                            j->warning(Category::Modbus, QString("Привязка равна null для регистра \"%1\"").arg(regName));
                                        continue;
                                    }

                                    // Проверяем, что привязка валидна
                                    if (!binding->isValid()) {
                                        if (auto* j = JournalAccess::journal())
                                            j->warning(Category::Modbus, QString("Привязка недействительна для регистра \"%1\"").arg(regName));
                                        continue;
                                    }

                                    if (binding->direction() != RegisterBinding::WriteOnly) {
                                        // Помечаем привязку как обновляемую, чтобы избежать рекурсии
                                        m_updatingBindings.insert(binding.get());
                                        binding->updateFromRegister(data);
                                        m_updatingBindings.remove(binding.get());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void BindingManager::onBindingValueChanged() {
    auto* sourceBinding = qobject_cast<RegisterBinding*>(sender());
    if (!sourceBinding || sourceBinding->direction() == RegisterBinding::ReadOnly) return;

    // Если привязка уже в процессе обновления, игнорируем
    if (m_updatingBindings.contains(sourceBinding)) {
        return;
    }

    QString regName = sourceBinding->registerName();
    QVector<quint16> data = sourceBinding->getRegisterData();

    auto regType = m_modbusClient->getRegisterType(m_serverAddress, regName);
    if (regType == QModbusDataUnit::DiscreteInputs || regType == QModbusDataUnit::InputRegisters) {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Modbus, QString("Невозможно записать в регистр только для чтения \"%1\"").arg(regName));
        return;
    }

    // Обновляем кэш
    updateRegisterCache(regName, data);

    // Обновляем все другие привязки этого регистра (кроме источника)
    if (m_bindings.contains(regName)) {
        for (auto& binding : m_bindings[regName]) {
            if (binding.get() != sourceBinding && binding->direction() != RegisterBinding::WriteOnly) {
                m_updatingBindings.insert(binding.get());
                binding->updateFromRegister(data);
                m_updatingBindings.remove(binding.get());
            }
        }
    }

    QVariant value;
    if (data.size() == 1) {
        value = QVariant(data[0]);
    } else {
        value = QVariant::fromValue(data);
    }
    emit bindingValueChanged(regName, value);

    // Отправляем запрос на запись в Modbus
    auto mdum = m_modbusClient->get_mdum();
    if (mdum.count(m_serverAddress) > 0) {
        if (mdum.at(m_serverAddress).count(regName) > 0) {
            if (m_registerToGroup.contains(regName)) {
                const QString& groupName = m_registerToGroup[regName];
                QModbusDataUnit unit = buildTransactionalUnit(groupName);
                if (unit.isValid()) {
                    m_modbusClient->writeMultipleRegisters(m_serverAddress, unit.startAddress(), unit.values());
                }
            } else {
                QModbusDataUnit unit = mdum.at(m_serverAddress).at(regName);
                unit.setValues(data);
                m_modbusClient->on_write(m_serverAddress, unit);
            }
        }
    }
}

void BindingManager::onRegisterValueRequested(const QString& regName, const QVector<quint16>& data) {
    emit registerValueRequested(regName, data);

    // Прямой запрос на запись регистра
    auto mdum = m_modbusClient->get_mdum();
    if (mdum.count(m_serverAddress) > 0) {
        if (mdum.at(m_serverAddress).count(regName) > 0) {
            // Также обновляем кэш и другие привязки
            updateRegisterCache(regName, data);

            if (m_registerToGroup.contains(regName)) {
                const QString& groupName = m_registerToGroup[regName];
                QModbusDataUnit unit = buildTransactionalUnit(groupName);
                if (unit.isValid()) {
                    m_modbusClient->writeMultipleRegisters(m_serverAddress, unit.startAddress(), unit.values());
                }
            } else {
                QModbusDataUnit unit = mdum.at(m_serverAddress).at(regName);
                unit.setValues(data);
                m_modbusClient->on_write(m_serverAddress, unit);
            }

            if (m_bindings.contains(regName)) {
                auto* sourceBinding = qobject_cast<RegisterBinding*>(sender());
                for (auto& binding : m_bindings[regName]) {
                    if (binding.get() != sourceBinding && binding->direction() != RegisterBinding::WriteOnly) {
                        m_updatingBindings.insert(binding.get());
                        binding->updateFromRegister(data);
                        m_updatingBindings.remove(binding.get());
                    }
                }
            }
        }
    }
}

void BindingManager::onReadRequested(const QString& regName) {
    emit readRequested(regName);

    requestRegisterRead(regName);
}

QList<QModbusDataUnit> BindingManager::getPollingUnits() const {
    QList<QModbusDataUnit> units;

    auto reg_v = m_modbusClient->get_reg_v(m_serverAddress);
    for (const auto& regMap : reg_v) {
        for (const auto& unit : regMap) {
            units.append(unit);
        }
    }

    return units;
}

void BindingManager::updateRegisterCache(const QString& regName, const QVector<quint16>& data) {
    m_registerCache[regName] = data;
}

QVector<quint16> BindingManager::getRegisterCache(const QString& regName) const {
    return m_registerCache.value(regName);
}

bool BindingManager::hasRegisterCache(const QString& regName) const {
    return m_registerCache.contains(regName);
}

void BindingManager::requestRegisterRead(const QString& regName) {
    QModbusDataUnit pollingUnit = findPollingUnitForRegister(regName);
    if (pollingUnit.isValid()) {
        m_modbusClient->sendSingleRequest(m_serverAddress, pollingUnit);
    }
}

void BindingManager::registerWidgetsFromContainer(QWidget* container) {
    if (!container) {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Modbus, "Не удалось зарегистрировать виджеты: контейнер равен null");
        return;
    }

    //qCDebug(logDebug()) << "Scanning container for bindings:" << container->objectName();

    // Рекурсивно сканируем все виджеты в контейнере
    scanWidgetForBindings(container);
}

void BindingManager::loadTransactionalGroups() {
    if (!m_modbusClient) return;

    auto groups = m_modbusClient->getTransactionalGroups(); // std::map<int, std::map<QString, QStringList>>
    if (groups.count(m_serverAddress) > 0) {
        const auto& serverGroups = groups.at(m_serverAddress);
        for (auto it = serverGroups.begin(); it != serverGroups.end(); ++it) {
            const QStringList& regNames = it->second;   // список регистров группы
            setTransactionalGroup(regNames);
        }
    }
}

void BindingManager::setTransactionalGroup(const QStringList& regNames) {
    if (regNames.size() < 2) return;

    // Создаём уникальное имя группы, например, объединяя имена регистров
    QString groupName = regNames.join('|');
    m_transactionalGroups[groupName] = regNames;
    for (const auto& reg : regNames) {
        m_registerToGroup[reg] = groupName;
    }
}

QModbusDataUnit BindingManager::findPollingUnitForRegister(const QString& regName) const {
    // Получаем список всех блоков для опроса
    //auto pollingUnits = getPollingUnits();

    auto mdum = m_modbusClient->get_mdum();
    if (mdum.count(m_serverAddress) > 0) {
        if (mdum.at(m_serverAddress).count(regName) > 0) {
            QModbusDataUnit unit = mdum.at(m_serverAddress).at(regName);
            return unit;
        }
    }

    return QModbusDataUnit();
}

QModbusDataUnit BindingManager::buildTransactionalUnit(const QString& groupName) const {
    auto mdum = m_modbusClient->get_mdum();
    const QStringList& regs = m_transactionalGroups[groupName];

    // Определяем минимальный адрес и полный размер
    int startAddress = std::numeric_limits<int>::max();
    unsigned int endAddress = 0;
    for (const auto& regName : regs) {
        if (mdum.count(m_serverAddress) && mdum.at(m_serverAddress).count(regName)) {
            const auto& unit = mdum.at(m_serverAddress).at(regName);
            startAddress = std::min(startAddress, unit.startAddress());
            endAddress = std::max(endAddress, (unsigned int)(unit.startAddress() + unit.valueCount()));
        }
    }

    int totalSize = endAddress - startAddress;
    QVector<quint16> values(totalSize, 0);

    // Заполняем данными из кэша
    for (const auto& regName : regs) {
        if (m_registerCache.contains(regName)) {
            const auto& data = m_registerCache[regName];
            if (mdum.at(m_serverAddress).count(regName)) {
                int offset = mdum.at(m_serverAddress).at(regName).startAddress() - startAddress;
                for (int i = 0; i < data.size(); ++i) {
                    values[offset + i] = data[i];
                }
            }
        }
    }

    return QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, values);
}

void BindingManager::scanWidgetForBindings(QWidget* widget) {
    if (!widget) return;

    // Проверяем сам виджет на наличие свойств для привязки
    QString regName = widget->property("modbusRegister").toString();
    if (!regName.isEmpty()) {
        createBindingForWidget(widget, regName);
    }

    // Сканируем все дочерние виджеты
    QList<QWidget*> childWidgets = widget->findChildren<QWidget*>(
        QString(), Qt::FindChildrenRecursively);

    for (QWidget* childWidget : childWidgets) {
        if (childWidget != widget) { // Избегаем повторной проверки самого себя
            QString childregName = childWidget->property("modbusRegister").toString();
            if (!childregName.isEmpty()) {
                createBindingForWidget(childWidget, childregName);
            }
        }
    }
}

void BindingManager::createBindingForWidget(QWidget* widget, const QString& regName) {
    if (!widget || regName.isEmpty()) return;

    // Получаем дополнительные свойства
    QString dataType = widget->property("modbusDataType").toString();
    QString directionStr = widget->property("modbusDirection").toString();
    RegisterBinding::BindingDirection direction = RegisterBinding::ReadWrite;

    if (!directionStr.isEmpty()) {
        direction = RegisterBinding::stringToDirection(directionStr);
    }

    // Создаем соответствующую привязку
    std::shared_ptr<RegisterBinding> binding = createAppropriateBinding(
        widget, regName, dataType);

    if (binding) {
        binding->setDirection(direction);
        addBinding(binding);

        // qCDebug(logDebug()) << "Created binding for widget:" << widget->objectName()
        //                     << "register:" << regName
        //                     << "type:" << dataType
        //                     << "direction:" << RegisterBinding::directionToString(direction);
    }
}

std::shared_ptr<RegisterBinding> BindingManager::createAppropriateBinding(
    QWidget* widget,
    const QString& regName,
    const QString& dataType) {

    //qCDebug(logDebug()) << "createAppropriateBinding" << regName;

    // Проверяем тип виджета и создаем соответствующую привязку
    if (qobject_cast<QCheckBox*>(widget)) {
        // Для QCheckBox - битовая привязка
        //qCDebug(logDebug()) << "QCheckBox" << regName;
        int bitPosition = widget->property("modbusBitPosition").toInt();
        return std::make_shared<CheckBoxBinding>(
            qobject_cast<QCheckBox*>(widget),
            regName,
            bitPosition,
            RegisterBinding::ReadWrite,  // По умолчанию
            BigEndian);                  // По умолчанию
    } else if (qobject_cast<QSpinBox*>(widget)) {
        //qCDebug(logDebug()) << "QSpinBox" << regName;
        return std::make_shared<SpinBoxBinding>(
            qobject_cast<QSpinBox*>(widget),
            regName,
            ModbusRegisterDataType_from_text(dataType.toStdString().c_str()),
            RegisterBinding::ReadWrite,  // По умолчанию
            BigEndian);                  // По умолчанию
    } else if (qobject_cast<QDoubleSpinBox*>(widget)) {
        //qCDebug(logDebug()) << "QDoubleSpinBox" << regName;
        return std::make_shared<DoubleSpinBoxBinding>(
            qobject_cast<QDoubleSpinBox*>(widget),
            regName,
            ModbusRegisterDataType_from_text(dataType.toStdString().c_str()),
            RegisterBinding::ReadWrite,  // По умолчанию
            BigEndian);                  // По умолчанию
    } else if (qobject_cast<QLineEdit*>(widget)) {
        //qCDebug(logDebug()) << "QLineEdit" << regName;
        return std::make_shared<LineEditBinding>(
            qobject_cast<QLineEdit*>(widget),
            regName);
    } else if (qobject_cast<QLabel*>(widget)) {
        //qCDebug(logDebug()) << "QLabel" << regName;
        return std::make_shared<LabelBinding>(
            qobject_cast<QLabel*>(widget),
            regName);
    }

    if (auto* j = JournalAccess::journal())
        j->warning(Category::Modbus, QString("Неподдерживаемый тип виджета для привязки: %1, регистр: %2")
                                         .arg(widget->metaObject()->className())
                                         .arg(regName));

    return nullptr;
}
