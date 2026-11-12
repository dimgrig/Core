#include "LabelBinding.h"

#include "modbus_functions.h"

LabelBinding::LabelBinding(QLabel* label,
                           const QString& registerName,
                           DisplayFormat format,
                           const QString& customFormat,
                           int precision,
                           BytesFormat modbus_format,
                           QObject* parent)
    : RegisterBinding(registerName, ReadOnly, modbus_format, parent) // Label всегда только для чтения
    , m_label(label)
    , m_format(format)
    , m_customFormat(customFormat)
    , m_precision(precision)
{
}

void LabelBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_label || data.isEmpty()) return;

    QString displayText;

    switch (m_format) {
    case Decimal:
        if (data.size() == 1) {
            displayText = QString::number(static_cast<int16_t>(data[0]));
        } else if (data.size() == 2) {
            int32_t value = usa_to_int((unsigned short*)&data[0], this->modbus_format());
            displayText = QString::number(value);
        }
        break;

    case Hexadecimal:
        if (data.size() == 1) {
            displayText = QString("0x%1").arg(data[0], 4, 16, QChar('0'));
        } else if (data.size() == 2) {
            uint32_t value = usa_to_uint((unsigned short*)&data[0], this->modbus_format());
            displayText = QString("0x%1").arg(value, 8, 16, QChar('0'));
        }
        break;

    case Binary:
        if (data.size() == 1) {
            displayText = QString::number(data[0], 2).rightJustified(16, '0');
        }
        break;

    case Float:
        if (data.size() == 2) {
            union { float f; quint32 i; } u;
            u.f = usa_to_float((unsigned short*)&data[0], this->modbus_format());
            displayText = QLocale::system().toString(static_cast<double>(u.f), 'f', m_precision);
        }
        break;

    case Double:
        if (data.size() == 4) {
            union { double d; quint64 i; } u;
            u.d = usa_to_double((unsigned short*)&data[0], this->modbus_format());
            displayText = QLocale::system().toString(u.d, 'f', m_precision);
        }
        break;

    case Custom:
        if (data.size() == 1) {
            displayText = m_customFormat.arg(data[0]);
        } else if (data.size() == 2) {
            int32_t value = usa_to_int((unsigned short*)&data[0], this->modbus_format());
            displayText = m_customFormat.arg(value);
        }
        break;

    case BitField:
        if (data.size() == 1) {
            QString bits;
            for (int i = 15; i >= 0; --i) {
                bits.append((data[0] & (1 << i)) ? '1' : '0');
                if (i % 4 == 0 && i != 0) bits.append(' ');
            }
            displayText = bits;
        }
        break;
    }

    if (!displayText.isEmpty()) {
        if (!m_customFormat.isEmpty()) {
            m_label->setText(m_customFormat.arg(displayText));
        } else {
            m_label->setText(displayText);
        }
    }
}

QVector<quint16> LabelBinding::getRegisterData() const
{
    // Label только для чтения, не возвращаем данных для записи
    return QVector<quint16>();
}

bool LabelBinding::isValid() const
{
    return m_label != nullptr;
}

void LabelBinding::setFormat(DisplayFormat format)
{
    m_format = format;
}

void LabelBinding::setCustomFormat(const QString& format)
{
    m_customFormat = format;
}

void LabelBinding::setPrecision(int precision)
{
    m_precision = qMax(0, precision);  // Ограничиваем неотрицательными значениями
}

