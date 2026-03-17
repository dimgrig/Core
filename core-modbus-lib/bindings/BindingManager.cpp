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

BindingManager::BindingManager(std::shared_ptr<ModbusClient> modbusClient, QObject* parent)
    : QObject(parent), m_modbusClient(modbusClient) {

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

void BindingManager::removeBinding(const QString& registerName) {
    if (m_bindings.contains(registerName)) {
        for (auto& binding : m_bindings[registerName]) {
            disconnect(binding.get(), nullptr, this, nullptr);
        }
        m_bindings.remove(registerName);
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

QList<std::shared_ptr<RegisterBinding>> BindingManager::get_bindings(const QString& registerName) const {
    if (m_bindings.contains(registerName)) {
        return m_bindings[registerName];
    } else {
        return {};
    }
}

void BindingManager::processModbusData(const QModbusDataUnit& unit) {
    auto mdum = m_modbusClient->get_mdum();

    for (const auto& regPair : mdum) {
        const QString& regName = regPair.first;
        const QModbusDataUnit& regUnit = regPair.second;

        if (regUnit.registerType() == unit.registerType()) {
            unsigned int start = regUnit.startAddress();
            unsigned int count = regUnit.valueCount();

            if ((start + count) > static_cast<unsigned int>(unit.startAddress()) &&
                start < (static_cast<unsigned int>(unit.startAddress()) + unit.valueCount())) {

                unsigned int offset = qMax((unsigned int)0, start - static_cast<unsigned int>(unit.startAddress()));
                unsigned int copyCount = qMin(count, (unit.valueCount() - offset));

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
                                qCDebug(logWarning()) << "Binding is null for register:" << regName;
                                continue;
                            }

                            // Проверяем, что привязка валидна
                            if (!binding->isValid()) {
                                qCDebug(logWarning()) << "Binding is invalid for register:" << regName;
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

void BindingManager::onBindingValueChanged() {
    auto* sourceBinding = qobject_cast<RegisterBinding*>(sender());
    if (!sourceBinding || sourceBinding->direction() == RegisterBinding::ReadOnly) return;

    // Если привязка уже в процессе обновления, игнорируем
    if (m_updatingBindings.contains(sourceBinding)) {
        return;
    }

    QString regName = sourceBinding->registerName();
    QVector<quint16> newData = sourceBinding->getRegisterData();

    // Обновляем кэш
    updateRegisterCache(regName, newData);

    // Обновляем все другие привязки этого регистра (кроме источника)
    if (m_bindings.contains(regName)) {
        for (auto& binding : m_bindings[regName]) {
            if (binding.get() != sourceBinding && binding->direction() != RegisterBinding::WriteOnly) {
                m_updatingBindings.insert(binding.get());
                binding->updateFromRegister(newData);
                m_updatingBindings.remove(binding.get());
            }
        }
    }

    QVariant value;
    if (newData.size() == 1) {
        value = QVariant(newData[0]);
    } else {
        value = QVariant::fromValue(newData);
    }
    emit bindingValueChanged(regName, value);

    // Отправляем запрос на запись в Modbus
    auto mdum = m_modbusClient->get_mdum();
    if (mdum.count(regName) > 0) {
        QModbusDataUnit unit = mdum.at(regName);
        unit.setValues(newData);
        m_modbusClient->on_write(unit);
    }
}

void BindingManager::onRegisterValueRequested(const QString& registerName, const QVector<quint16>& data) {
    emit registerValueRequested(registerName, data);

    // Прямой запрос на запись регистра
    auto mdum = m_modbusClient->get_mdum();
    if (mdum.count(registerName) > 0) {
        QModbusDataUnit unit = mdum.at(registerName);
        unit.setValues(data);
        m_modbusClient->on_write(unit);

        // Также обновляем кэш и другие привязки
        updateRegisterCache(registerName, data);

        if (m_bindings.contains(registerName)) {
            auto* sourceBinding = qobject_cast<RegisterBinding*>(sender());
            for (auto& binding : m_bindings[registerName]) {
                if (binding.get() != sourceBinding && binding->direction() != RegisterBinding::WriteOnly) {
                    m_updatingBindings.insert(binding.get());
                    binding->updateFromRegister(data);
                    m_updatingBindings.remove(binding.get());
                }
            }
        }
    }
}

void BindingManager::onReadRequested(const QString& registerName) {
    emit readRequested(registerName);

    requestRegisterRead(registerName);
}

QList<QModbusDataUnit> BindingManager::getPollingUnits() const {
    QList<QModbusDataUnit> units;

    auto reg_v = m_modbusClient->get_reg_v();
    for (const auto& regMap : reg_v) {
        for (const auto& unit : regMap) {
            units.append(unit);
        }
    }

    return units;
}

void BindingManager::updateRegisterCache(const QString& registerName, const QVector<quint16>& data) {
    m_registerCache[registerName] = data;
}

QVector<quint16> BindingManager::getRegisterCache(const QString& registerName) const {
    return m_registerCache.value(registerName);
}

bool BindingManager::hasRegisterCache(const QString& registerName) const {
    return m_registerCache.contains(registerName);
}

void BindingManager::requestRegisterRead(const QString& registerName) {
    QModbusDataUnit pollingUnit = findPollingUnitForRegister(registerName);
    if (pollingUnit.isValid()) {
        m_modbusClient->sendSingleRequest(pollingUnit);
    }
}

void BindingManager::registerWidgetsFromContainer(QWidget* container) {
    if (!container) {
        qCWarning(logWarning()) << "Cannot register widgets: container is null";
        return;
    }

    //qCDebug(logDebug()) << "Scanning container for bindings:" << container->objectName();

    // Рекурсивно сканируем все виджеты в контейнере
    scanWidgetForBindings(container);
}

QModbusDataUnit BindingManager::findPollingUnitForRegister(const QString& registerName) const {
    // Получаем список всех блоков для опроса
    auto pollingUnits = getPollingUnits();

    // Получаем информацию о конкретном регистре из ModbusClient
    auto mdum = m_modbusClient->get_mdum();
    if (!mdum.count(registerName)) {
        return QModbusDataUnit();
    }

    QModbusDataUnit registerUnit = mdum.at(registerName);
    return registerUnit;
}

void BindingManager::scanWidgetForBindings(QWidget* widget) {
    if (!widget) return;

    // Проверяем сам виджет на наличие свойств для привязки
    QString registerName = widget->property("modbusRegister").toString();
    if (!registerName.isEmpty()) {
        createBindingForWidget(widget, registerName);
    }

    // Сканируем все дочерние виджеты
    QList<QWidget*> childWidgets = widget->findChildren<QWidget*>(
        QString(), Qt::FindChildrenRecursively);

    for (QWidget* childWidget : childWidgets) {
        if (childWidget != widget) { // Избегаем повторной проверки самого себя
            QString childRegisterName = childWidget->property("modbusRegister").toString();
            if (!childRegisterName.isEmpty()) {
                createBindingForWidget(childWidget, childRegisterName);
            }
        }
    }
}

void BindingManager::createBindingForWidget(QWidget* widget, const QString& registerName) {
    if (!widget || registerName.isEmpty()) return;

    // Получаем дополнительные свойства
    QString dataType = widget->property("modbusDataType").toString();
    QString directionStr = widget->property("modbusDirection").toString();
    RegisterBinding::BindingDirection direction = RegisterBinding::ReadWrite;

    if (!directionStr.isEmpty()) {
        direction = RegisterBinding::stringToDirection(directionStr);
    }

    // Создаем соответствующую привязку
    std::shared_ptr<RegisterBinding> binding = createAppropriateBinding(
        widget, registerName, dataType);

    if (binding) {
        binding->setDirection(direction);
        addBinding(binding);

        // qCDebug(logDebug()) << "Created binding for widget:" << widget->objectName()
        //                     << "register:" << registerName
        //                     << "type:" << dataType
        //                     << "direction:" << RegisterBinding::directionToString(direction);
    }
}

std::shared_ptr<RegisterBinding> BindingManager::createAppropriateBinding(
    QWidget* widget,
    const QString& registerName,
    const QString& dataType) {

    //qCDebug(logDebug()) << "createAppropriateBinding" << registerName;

    // Проверяем тип виджета и создаем соответствующую привязку
    if (qobject_cast<QCheckBox*>(widget)) {
        // Для QCheckBox - битовая привязка
        //qCDebug(logDebug()) << "QCheckBox" << registerName;
        int bitPosition = widget->property("modbusBitPosition").toInt();
        return std::make_shared<CheckBoxBinding>(
            qobject_cast<QCheckBox*>(widget),
            registerName,
            bitPosition,
            RegisterBinding::ReadWrite,  // По умолчанию
            BigEndian);                  // По умолчанию
    } else if (qobject_cast<QSpinBox*>(widget)) {
        //qCDebug(logDebug()) << "QSpinBox" << registerName;
        return std::make_shared<SpinBoxBinding>(
            qobject_cast<QSpinBox*>(widget),
            registerName,
            ModbusRegisterDataType_from_text(dataType.toStdString().c_str()),
            RegisterBinding::ReadWrite,  // По умолчанию
            BigEndian);                  // По умолчанию
    } else if (qobject_cast<QDoubleSpinBox*>(widget)) {
        //qCDebug(logDebug()) << "QDoubleSpinBox" << registerName;
        return std::make_shared<DoubleSpinBoxBinding>(
            qobject_cast<QDoubleSpinBox*>(widget),
            registerName,
            ModbusRegisterDataType_from_text(dataType.toStdString().c_str()),
            RegisterBinding::ReadWrite,  // По умолчанию
            BigEndian);                  // По умолчанию
    } else if (qobject_cast<QLineEdit*>(widget)) {
        //qCDebug(logDebug()) << "QLineEdit" << registerName;
        return std::make_shared<LineEditBinding>(
            qobject_cast<QLineEdit*>(widget),
            registerName);
    } else if (qobject_cast<QLabel*>(widget)) {
        //qCDebug(logDebug()) << "QLabel" << registerName;
        return std::make_shared<LabelBinding>(
            qobject_cast<QLabel*>(widget),
            registerName);
    }

    qCWarning(logWarning()) << "Unsupported widget type for binding:"
                            << widget->metaObject()->className()
                            << "register:" << registerName;

    return nullptr;
}
