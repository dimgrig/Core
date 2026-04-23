#ifndef BITAWAREBINDINGMANAGER_H
#define BITAWAREBINDINGMANAGER_H

#include "BindingManager.h"
#include "ModbusClient.h"

#include "CheckBoxBinding.h"

class CORE_MODBUS_LIB_EXPORT BitAwareBindingManager : public BindingManager {
    Q_OBJECT
public:
    explicit BitAwareBindingManager(int serverAddress,
                                    std::shared_ptr<ModbusClient> modbusClient,
                                    QObject* parent = nullptr);

    // Переопределяем обработчик для правильной работы с битовыми привязками
    void onBindingValueChanged() override;

private:
    // Вспомогательные методы для работы с битами
    quint16 applyBitChanges(const QString& registerName, CheckBoxBinding* sourceBinding);
    QVector<std::shared_ptr<CheckBoxBinding>> getCheckBoxBindings(const QString& registerName) const;
};

#endif // BITAWAREBINDINGMANAGER_H
