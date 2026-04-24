#ifndef DOUBLESPINBOXBINDING_H
#define DOUBLESPINBOXBINDING_H

#include <QDoubleSpinBox>
#include <QModbusDataUnit>
#include <QObject>

#include "ModbusBase.h"
#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT DoubleSpinBoxBinding : public RegisterBinding {
    Q_OBJECT
public:
    DoubleSpinBoxBinding(QDoubleSpinBox* spinBox,
                         const QString& registerName,
                         ModbusRegisterDataType dataType = ModbusRegisterDataType_float,
                         BindingDirection direction = ReadWrite,
                         BytesFormat modbus_format = BigEndian,
                         QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    ModbusRegisterDataType dataType() const { return m_dataType; }
    void setDataType(ModbusRegisterDataType type) { m_dataType = type; }

private slots:
    void onValueChanged(double value);

private:
    QDoubleSpinBox* m_spinBox;
    ModbusRegisterDataType m_dataType;
};
#endif // DOUBLESPINBOXBINDING_H
