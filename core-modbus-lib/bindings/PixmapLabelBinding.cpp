#include "PixmapLabelBinding.h"

#include "Logger.h"

PixmapLabelBinding::PixmapLabelBinding(QLabel* label,
                                       const QString& registerName,
                                       int bitPosition,
                                       const QPixmap& pixmapOn,
                                       const QPixmap& pixmapOff,
                                       BindingDirection direction,
                                       BytesFormat modbus_format,
                                       QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_label(label)
    , m_bitPosition(bitPosition)
    , m_useBit(true)
    , m_pixmapOn(pixmapOn)
    , m_pixmapOff(pixmapOff)
{
}

PixmapLabelBinding::PixmapLabelBinding(QLabel* label,
                                       const QString& registerName,
                                       const QMap<quint16, QPixmap>& statePixmaps,
                                       BindingDirection direction,
                                       BytesFormat modbus_format,
                                       QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_label(label)
    , m_bitPosition(-1)
    , m_useBit(false)
    , m_statePixmaps(statePixmaps)
{
}

PixmapLabelBinding::PixmapLabelBinding(QLabel* label,
                                       const QString& registerName,
                                       const QMap<quint16, QPixmap>& valuePixmaps,
                                       bool useBit,
                                       BindingDirection direction,
                                       BytesFormat modbus_format,
                                       QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_label(label)
    , m_bitPosition(-1)
    , m_useBit(useBit)
    , m_statePixmaps(valuePixmaps)
{
}

void PixmapLabelBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_label || data.isEmpty()) return;

    if (m_useBit && m_bitPosition >= 0 && m_bitPosition < 16) {
        // Режим бита - извлекаем конкретный бит
        quint16 registerValue = data.first();
        bool bitState = (registerValue >> m_bitPosition) & 0x01;

        // Устанавливаем соответствующее изображение
        if (bitState && !m_pixmapOn.isNull()) {
            m_label->setPixmap(m_pixmapOn);
        } else if (!bitState && !m_pixmapOff.isNull()) {
            m_label->setPixmap(m_pixmapOff);
        }

        // qCDebug(logDebug()) << "PixmapLabelBinding: Bit update" << registerName()
        //                     << "bit" << m_bitPosition << "=" << bitState;

    } else if (!m_statePixmaps.isEmpty()) {
        // Режим значений - используем карту состояний
        quint16 value = data.first();

        if (m_statePixmaps.contains(value)) {
            m_label->setPixmap(m_statePixmaps[value]);
        } else {
            // Если значение не найдено, используем изображение по умолчанию (первое)
            if (!m_statePixmaps.isEmpty()) {
                m_label->setPixmap(m_statePixmaps.first());
            }
        }

        // qCDebug(logDebug()) << "PixmapLabelBinding: Value update" << registerName()
        //                     << "value:" << value;
    }
}

QVector<quint16> PixmapLabelBinding::getRegisterData() const
{
    // PixmapLabelBinding обычно только для чтения
    return QVector<quint16>();
}

bool PixmapLabelBinding::isValid() const
{
    if (!m_label) return false;

    if (m_useBit) {
        return m_bitPosition >= 0 && m_bitPosition < 16 &&
               (!m_pixmapOn.isNull() || !m_pixmapOff.isNull());
    } else {
        return !m_statePixmaps.isEmpty();
    }
}

void PixmapLabelBinding::setPixmapForState(quint16 value, const QPixmap& pixmap)
{
    m_statePixmaps[value] = pixmap;
}

void PixmapLabelBinding::setPixmapForBitState(bool state, const QPixmap& pixmap)
{
    if (state) {
        m_pixmapOn = pixmap;
    } else {
        m_pixmapOff = pixmap;
    }
}

void PixmapLabelBinding::clearPixmaps()
{
    m_statePixmaps.clear();
}
