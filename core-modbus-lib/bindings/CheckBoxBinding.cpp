#include "CheckBoxBinding.h"

#include "Logger.h"

CheckBoxBinding::CheckBoxBinding(QCheckBox* checkBox,
                                 const QString& registerName,
                                 int bitPosition,
                                 BindingDirection direction,
                                 BytesFormat modbus_format,
                                 QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_checkBox(checkBox)
    , m_bitPosition(bitPosition)
{
    if (m_checkBox) {
        if (direction != ReadOnly) {
            connect(m_checkBox, &QCheckBox::stateChanged,
                    this, &CheckBoxBinding::onStateChanged);
        }
        m_checkBox->setEnabled(direction != ReadOnly);
    }
}

void CheckBoxBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_checkBox || data.isEmpty()) return;

    if (m_bitPosition >= 0 && m_bitPosition < 16) {
        quint16 registerValue = data.first();
        bool bitValue = (registerValue >> m_bitPosition) & 0x01;

        m_checkBox->blockSignals(true);
        auto previous = m_checkBox->isChecked();
        m_checkBox->setChecked(bitValue);
        if (previous != bitValue) {
            emit valueChanged();
        }
        m_checkBox->blockSignals(false);

        // qCDebug(logDebug()) << "CheckBoxBinding: Update" << registerName()
        //                     << "bit" << m_bitPosition << "=" << bitValue;
    }
}

QVector<quint16> CheckBoxBinding::getRegisterData() const
{
    // Этот метод не должен использоваться напрямую для записи в BindingManager
    // Вместо этого мы должны получать текущее значение из кэша и изменять только нужный бит
    qDebug(logWarning()) << "CheckBoxBinding::getRegisterData should not be called directly";
    return QVector<quint16>(1, 0);
}

// Новый метод для получения данных с учетом текущего значения регистра
QVector<quint16> CheckBoxBinding::getRegisterDataWithMask(quint16 currentValue) const
{
    if (!m_checkBox) return QVector<quint16>(1, currentValue);

    quint16 newValue = currentValue;

    // Изменяем только нужный бит
    if (m_checkBox->isChecked()) {
        newValue |= (1 << m_bitPosition);    // Устанавливаем бит
    } else {
        newValue &= ~(1 << m_bitPosition);   // Сбрасываем бит
    }

    // qCDebug(logDebug()) << "CheckBoxBinding: Bit change" << registerName()
    //                     << "current:" << currentValue << "new:" << newValue
    //                     << "bit" << m_bitPosition;

    return QVector<quint16>(1, newValue);
}

bool CheckBoxBinding::isValid() const
{
    return m_checkBox != nullptr &&
           m_bitPosition >= 0 &&
           m_bitPosition < 16;
}

void CheckBoxBinding::onStateChanged(int state)
{
    Q_UNUSED(state)

    if (direction() != ReadOnly) {
        emit valueChanged(); // Используем valueChanged вместо registerValueRequested
    }
}

int CheckBoxBinding::bitPosition() const
{
    return m_bitPosition;
}

bool CheckBoxBinding::isChecked() const {
    return m_checkBox->isChecked();
}

void CheckBoxBinding::setChecked(bool checked) {
    return m_checkBox->setChecked(checked);
}
