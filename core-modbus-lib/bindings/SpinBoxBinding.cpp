#include "SpinBoxBinding.h"

#include "ModbusDataConverter.h"

SpinBoxBinding::SpinBoxBinding(QSpinBox* spinBox,
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
            connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, &SpinBoxBinding::onValueChanged);
        }

        // Блокируем если только для чтения
        m_spinBox->setEnabled(direction != ReadOnly);
    }
}

void SpinBoxBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_spinBox || data.isEmpty()) return;

    QVariant v = ModbusDataConverter::registersToVariant(data, m_registerName, ModbusRegisterDataType_to_text(m_dataType), m_modbus_format);
    long long value = v.toLongLong();

    // Блокируем сигналы чтобы избежать рекурсии
    m_spinBox->blockSignals(true);
    auto previous = m_spinBox->value();
    m_spinBox->setValue(value);
    if (previous != value) {
        emit valueChanged();
    }
    m_spinBox->blockSignals(false);
}

QVector<quint16> SpinBoxBinding::getRegisterData() const
{
    if (!m_spinBox) return QVector<quint16>();

    long long value = m_spinBox->value();

    return ModbusDataConverter::variantToRegisters(value, m_registerName, ModbusRegisterDataType_to_text(m_dataType), m_modbus_format);
}

bool SpinBoxBinding::isValid() const
{
    return m_spinBox != nullptr;
}

void SpinBoxBinding::onValueChanged(int value)
{
    Q_UNUSED(value)

    if (direction() != ReadOnly) {
        emit registerValueRequested(registerName(), getRegisterData());
    }
}

int SpinBoxBinding::value() const {
    return m_spinBox->value();
}
