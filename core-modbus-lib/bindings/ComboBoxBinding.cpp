#include "ComboBoxBinding.h"

#include "Logger.h"

ComboBoxBinding::ComboBoxBinding(QComboBox* comboBox,
                                 const QString& registerName,
                                 BindingDirection direction,
                                 BytesFormat modbus_format,
                                 QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_comboBox(comboBox)
    , m_useValueMap(false)
    , m_bitMode(false)
    , m_bitPosition(-1)
    , m_expectedSize(1)
{
    if (m_comboBox) {
        if (direction != ReadOnly) {
            connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &ComboBoxBinding::onCurrentIndexChanged);
        }
        m_comboBox->setEnabled(direction != ReadOnly);
    }
}

ComboBoxBinding::ComboBoxBinding(QComboBox* comboBox,
                                 const QString& registerName,
                                 const QMap<quint16, int>& valueToIndexMap,
                                 BindingDirection direction,
                                 BytesFormat modbus_format,
                                 QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_comboBox(comboBox)
    , m_valueToIndexMap(valueToIndexMap)
    , m_useValueMap(true)
    , m_bitMode(false)
    , m_bitPosition(-1)
    , m_expectedSize(1)
{
    if (m_comboBox) {
        if (direction != ReadOnly) {
            connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &ComboBoxBinding::onCurrentIndexChanged);
        }
        m_comboBox->setEnabled(direction != ReadOnly);
    }
}

ComboBoxBinding::ComboBoxBinding(QComboBox* comboBox,
                                 const QString& registerName,
                                 int bitPosition,
                                 BindingDirection direction,
                                 BytesFormat modbus_format,
                                 QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)
    , m_comboBox(comboBox)
    , m_useValueMap(false)
    , m_bitMode(true)
    , m_bitPosition(bitPosition)
    , m_expectedSize(1)
{
    if (m_comboBox) {
        if (direction != ReadOnly) {
            connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &ComboBoxBinding::onCurrentIndexChanged);
        }
        m_comboBox->setEnabled(direction != ReadOnly);
    }
}

void ComboBoxBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_comboBox || data.isEmpty()) return;

    // Не обновляем, пока пользователь редактирует поле
    if (m_comboBox->hasFocus())
        return;

    quint16 registerValue = data.first();
    int index = 0;

    if (m_bitMode && m_bitPosition >= 0 && m_bitPosition < 16) {
        // Режим бита - извлекаем бит и используем как индекс (0 или 1)
        bool bitValue = (registerValue >> m_bitPosition) & 0x01;
        index = bitValue ? 1 : 0;

        // Проверяем, что в комбобоксе достаточно элементов
        if (m_comboBox->count() < 2) {
            qWarning() << "ComboBoxBinding: Not enough items in comboBox for bit mode";
            return;
        }
    } else if (m_useValueMap) {
        // Используем пользовательское отображение
        index = m_valueToIndexMap.value(registerValue, 0);
    } else {
        // Прямое соответствие - значение регистра как индекс
        index = static_cast<int>(registerValue);

        // Ограничиваем индекс размером комбобокса
        if (index >= m_comboBox->count()) {
            index = m_comboBox->count() - 1;
        }
        if (index < 0) {
            index = 0;
        }
    }

    // Блокируем сигналы чтобы избежать рекурсии
    m_comboBox->blockSignals(true);
    auto previous = m_comboBox->currentIndex();
    m_comboBox->setCurrentIndex(index);
    if (previous != index) {
        emit valueChanged();
    }
    m_comboBox->blockSignals(false);

    // qCDebug(logDebug()) << "ComboBoxBinding: Update" << registerName()
    //                     << "register value:" << registerValue
    //                     << "combo index:" << index;
}

QVector<quint16> ComboBoxBinding::getRegisterData() const
{
    if (!m_comboBox) return QVector<quint16>();

    int currentIndex = m_comboBox->currentIndex();
    quint16 value = getRegisterValueFromIndex(currentIndex);

    QVector<quint16> result(m_expectedSize, 0);
    result[0] = value;

    // qCDebug(logDebug()) << "ComboBoxBinding: Writing" << registerName()
    //                     << "combo index:" << currentIndex
    //                     << "register value:" << value;

    return result;
}

bool ComboBoxBinding::isValid() const
{
    if (!m_comboBox) return false;

    if (m_bitMode) {
        return m_bitPosition >= 0 && m_bitPosition < 16 && m_comboBox->count() >= 2;
    }

    return true;
}

void ComboBoxBinding::onCurrentIndexChanged(int index)
{
    Q_UNUSED(index)

    if (direction() != ReadOnly) {
        emit registerValueRequested(registerName(), getRegisterData());
    }
}

quint16 ComboBoxBinding::getRegisterValueFromIndex(int index) const
{
    if (m_bitMode && m_bitPosition >= 0 && m_bitPosition < 16) {
        // В режиме бита: индекс 0 = бит 0, индекс 1 = бит 1
        // Возвращаем маску с установленным битом
        return (index > 0) ? (1 << m_bitPosition) : 0;
    } else if (m_useValueMap) {
        // Ищем значение регистра по индексу (обратное отображение)
        for (auto it = m_valueToIndexMap.begin(); it != m_valueToIndexMap.end(); ++it) {
            if (it.value() == index) {
                return it.key();
            }
        }
        return m_valueToIndexMap.firstKey(); // значение по умолчанию
    } else {
        // Прямое соответствие - индекс как значение регистра
        return static_cast<quint16>(index);
    }
}

int ComboBoxBinding::getIndexFromRegisterValue(quint16 value) const
{
    if (m_bitMode && m_bitPosition >= 0 && m_bitPosition < 16) {
        // Режим бита - извлекаем бит
        return (value >> m_bitPosition) & 0x01;
    } else if (m_useValueMap) {
        // Используем пользовательское отображение
        return m_valueToIndexMap.value(value, 0);
    } else {
        // Прямое соответствие
        return static_cast<int>(value);
    }
}

void ComboBoxBinding::setValueMap(const QMap<quint16, int>& valueToIndexMap)
{
    m_valueToIndexMap = valueToIndexMap;
    m_useValueMap = true;
}

void ComboBoxBinding::setBitMode(bool bitMode, int bitPosition)
{
    m_bitMode = bitMode;
    if (bitMode) {
        m_bitPosition = bitPosition;
    }
}
