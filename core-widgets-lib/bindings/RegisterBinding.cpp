#include "RegisterBinding.h"

RegisterBinding::RegisterBinding(const QString& registerName,
                                 BindingDirection direction,
                                 BytesFormat modbus_format,
                                 QObject* parent)
    : QObject(parent)
    , m_registerName(registerName)
    , m_direction(direction)
    , m_modbus_format(modbus_format)
{
}

RegisterBinding::~RegisterBinding()
{
    // Базовый деструктор
}

// Базовые реализации (можно переопределить в производных классах)
void RegisterBinding::updateFromRegister(const QVector<quint16>& data)
{
    Q_UNUSED(data)
    //qCDebug(logDebug()) << "Base updateFromRegister called for register:" << m_registerName;
}

QVector<quint16> RegisterBinding::getRegisterData() const
{
    //qCDebug(logDebug()) << "Base getRegisterData called for register:" << m_registerName;
    return QVector<quint16>();
}

bool RegisterBinding::isValid() const
{
    return !m_registerName.isEmpty();
}

// Дополнительные методы для удобства
void RegisterBinding::setRegisterName(const QString& name)
{
    if (m_registerName != name) {
        m_registerName = name;
        // Можно добавить сигнал, если нужно уведомлять об изменении имени
    }
}

void RegisterBinding::setDirection(BindingDirection direction)
{
    if (m_direction != direction) {
        m_direction = direction;
        // Можно добавить сигнал, если нужно уведомлять об изменении направления
    }
}

void RegisterBinding::setBytesFormat(BytesFormat modbus_format)
{
    if (m_modbus_format != modbus_format) {
        m_modbus_format = modbus_format;
        // Можно добавить сигнал, если нужно уведомлять об изменении направления
    }
}

void RegisterBinding::requestRead()
{
    if (m_direction != WriteOnly) {
        emit readRequested(m_registerName);
    }
}

// Статические вспомогательные методы
QString RegisterBinding::directionToString(BindingDirection direction)
{
    switch (direction) {
    case ReadOnly:
        return "ReadOnly";
    case WriteOnly:
        return "WriteOnly";
    case ReadWrite:
        return "ReadWrite";
    default:
        return "Unknown";
    }
}

RegisterBinding::BindingDirection RegisterBinding::stringToDirection(const QString& directionStr)
{
    if (directionStr.compare("ReadOnly", Qt::CaseInsensitive) == 0) {
        return ReadOnly;
    } else if (directionStr.compare("WriteOnly", Qt::CaseInsensitive) == 0) {
        return WriteOnly;
    } else {
        return ReadWrite; // По умолчанию
    }
}
