#ifndef SPINBOXBINDING_H
#define SPINBOXBINDING_H

#include <QModbusDataUnit>
#include <QObject>
#include <QSpinBox>

#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT SpinBoxBinding : public RegisterBinding {
    Q_OBJECT
public:
    SpinBoxBinding(QSpinBox* spinBox,
                   const QString& registerName,
                   ModbusRegisterDataType dataType = ModbusRegisterDataType_ushort,
                   BindingDirection direction = ReadWrite,
                   BytesFormat modbus_format = BigEndian,
                   QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    ModbusRegisterDataType dataType() const { return m_dataType; }
    void setDataType(ModbusRegisterDataType type) { m_dataType = type; }

    int value() const;
private slots:
    void onValueChanged(int value);

private:
    QSpinBox* m_spinBox;
    ModbusRegisterDataType m_dataType;
};

#endif // SPINBOXBINDING_H
