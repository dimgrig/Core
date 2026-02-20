#include "BitAwareBindingManager.h"

#include "Logger.h"

BitAwareBindingManager::BitAwareBindingManager(std::shared_ptr<ModbusClient> modbusClient, QObject* parent)
    : BindingManager(modbusClient, parent) {
}

void BitAwareBindingManager::onBindingValueChanged() {
    auto* sourceCheckBox = qobject_cast<CheckBoxBinding*>(sender());
    if (!sourceCheckBox || sourceCheckBox->direction() == RegisterBinding::ReadOnly) {
        BindingManager::onBindingValueChanged();
        return;
    }

    QString regName = sourceCheckBox->registerName();

    // Если привязка уже в процессе обновления, игнорируем
    if (m_updatingBindings.contains(sourceCheckBox)) {
        return;
    }

    // Получаем текущее значение регистра из кэша
    QVector<quint16> currentData = getRegisterCache(regName);
    if (currentData.isEmpty()) {
        currentData = QVector<quint16>(1, 0);
    }

    // Применяем изменения от всех CheckBox привязок этого регистра
    quint16 newValue = applyBitChanges(regName, sourceCheckBox);
    QVector<quint16> newData(1, newValue);

    // Обновляем кэш
    updateRegisterCache(regName, newData);

    // Обновляем все другие привязки этого регистра
    if (m_bindings.contains(regName)) {
        for (auto& binding : m_bindings[regName]) {
            if (binding.get() != sourceCheckBox && binding->direction() != RegisterBinding::WriteOnly) {
                m_updatingBindings.insert(binding.get());
                binding->updateFromRegister(newData);
                m_updatingBindings.remove(binding.get());
            }
        }
    }

    // Отправляем запрос на запись в Modbus
    auto mdum = m_modbusClient->get_mdum();
    if (mdum.count(regName) > 0) {
        QModbusDataUnit unit = mdum.at(regName);
        unit.setValues(newData);
        m_modbusClient->on_write(unit);
    }
}

quint16 BitAwareBindingManager::applyBitChanges(const QString& registerName, CheckBoxBinding* sourceBinding) {
    QVector<quint16> currentData = getRegisterCache(registerName);
    quint16 currentValue = currentData.isEmpty() ? 0 : currentData.first();

    // Получаем все CheckBox привязки для этого регистра
    auto checkBoxBindings = getCheckBoxBindings(registerName);

    // Применяем изменения от sourceBinding
    if (sourceBinding) {
        QVector<quint16> newData = sourceBinding->getRegisterDataWithMask(currentValue);
        if (!newData.isEmpty()) {
            currentValue = newData.first();
        }
    }

    return currentValue;
}

QVector<std::shared_ptr<CheckBoxBinding>> BitAwareBindingManager::getCheckBoxBindings(const QString& registerName) const {
    QVector<std::shared_ptr<CheckBoxBinding>> result;

    if (m_bindings.contains(registerName)) {
        for (auto& binding : m_bindings[registerName]) {
            auto checkBoxBinding = std::dynamic_pointer_cast<CheckBoxBinding>(binding);
            if (checkBoxBinding) {
                result.append(checkBoxBinding);
            }
        }
    }

    return result;
}
