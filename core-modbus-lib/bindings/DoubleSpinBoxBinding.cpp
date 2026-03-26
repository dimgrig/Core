#include "DoubleSpinBoxBinding.h"

#include <cstdint>

#include "ModbusDataConverter.h"

namespace {
union FloatUnion {
    float f;
    quint32 i;
};

union DoubleUnion {
    double d;
    quint64 i;
};
}

DoubleSpinBoxBinding::DoubleSpinBoxBinding(QDoubleSpinBox* spinBox,
                                           const QString& registerName,
                                           ModbusRegisterDataType dataType,
                                           BindingDirection direction,
                                           BytesFormat modbus_format,
                                           QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_spinBox(spinBox)
    , m_dataType(dataType)
{
    if (m_spinBox) {
        // Для записи подключаем сигнал изменения значения
        if (direction != ReadOnly) {
            connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &DoubleSpinBoxBinding::onValueChanged);
        }

        // Блокируем если только для чтения
        m_spinBox->setEnabled(direction != ReadOnly);
    }
}

void DoubleSpinBoxBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_spinBox || data.isEmpty()) return;

    QVariant v = ModbusDataConverter::registersToVariant(data, m_registerName, ModbusRegisterDataType_to_text(m_dataType), m_modbus_format);
    double value = v.toDouble();

    // Блокируем сигналы чтобы избежать рекурсии
    m_spinBox->blockSignals(true);
    auto previous = m_spinBox->value();
    m_spinBox->setValue(value);
    if (!(std::abs(previous - value) < 0.0001)) { //if (previous != value) {
        emit valueChanged();
    }
    m_spinBox->blockSignals(false);
}

QVector<quint16> DoubleSpinBoxBinding::getRegisterData() const
{
    if (!m_spinBox) return QVector<quint16>();

    double value = m_spinBox->value();

    return ModbusDataConverter::variantToRegisters(value, m_registerName, ModbusRegisterDataType_to_text(m_dataType), m_modbus_format);
}

bool DoubleSpinBoxBinding::isValid() const
{
    return m_spinBox != nullptr;
}

void DoubleSpinBoxBinding::onValueChanged(double value)
{
    Q_UNUSED(value)

    if (direction() != ReadOnly) {
        emit registerValueRequested(registerName(), getRegisterData());
    }
}
