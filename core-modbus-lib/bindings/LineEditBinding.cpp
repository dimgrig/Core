#include "LineEditBinding.h"

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif

#include "Logger.h"

#include "Categories.h"
#include "JournalAccess.h"

LineEditBinding::LineEditBinding(QLineEdit* lineEdit,
                                 const QString& registerName,
                                 int stringLength,
                                 StringFormat format,
                                 QString encoding,
                                 BindingDirection direction,
                                 BytesFormat modbus_format,
                                 QObject* parent)
    : RegisterBinding(registerName, direction, modbus_format, parent)  // Для строк обычно BigEndian
    , m_lineEdit(lineEdit)
    , m_stringLength(stringLength)
    , m_format(format)
    , m_encoding(encoding.toUpper())
    , m_textChanged(false)
{
    if (m_lineEdit) {
        // Для записи подключаем сигналы
        if (direction != ReadOnly) {
            // Отслеживаем изменения текста
            connect(m_lineEdit, &QLineEdit::textChanged,
                    this, &LineEditBinding::onTextChanged);

            // Запись при завершении редактирования (потере фокуса или Enter)
            connect(m_lineEdit, &QLineEdit::editingFinished,
                    this, &LineEditBinding::onEditingFinished);
        }

        // Блокируем если только для чтения
        m_lineEdit->setEnabled(direction != ReadOnly);

        // Устанавливаем максимальную длину для FixedLength формата
        if (m_format == FixedLength) {
            m_lineEdit->setMaxLength(m_stringLength);
        }
    }
}

void LineEditBinding::updateFromRegister(const QVector<quint16>& data)
{
    if (!m_lineEdit) return;

    // Не обновляем, пока пользователь редактирует поле
    if (m_lineEdit->hasFocus())
        return;

    QString text = decodeString(data);

    // Блокируем сигналы чтобы избежать рекурсии
    m_lineEdit->blockSignals(true);
    m_lineEdit->setText(text);
    m_textChanged = false; // Сбрасываем флаг изменения
    emit valueChanged();
    m_lineEdit->blockSignals(false);
}

QVector<quint16> LineEditBinding::getRegisterData() const
{
    if (!m_lineEdit) return QVector<quint16>();

    return encodeString(m_lineEdit->text());
}

bool LineEditBinding::isValid() const
{
    return m_lineEdit != nullptr;
}

void LineEditBinding::onTextChanged(const QString& text)
{
    Q_UNUSED(text)
    if (direction() != ReadOnly) {
        m_textChanged = true;
    }
}

void LineEditBinding::onEditingFinished()
{
    if (m_textChanged && direction() != ReadOnly) {
        emit registerValueRequested(registerName(), getRegisterData());
        m_textChanged = false;
    }
}

QString LineEditBinding::decodeString(const QVector<quint16>& data) const
{
    if (data.isEmpty()) return QString();

    QByteArray bytes;
    int byteCount = data.size() * 2; // Каждый регистр - 2 байта

    // Преобразуем регистры в байты с учетом порядка байт
    for (quint16 reg : data) {
        if (modbus_format() == BigEndian) {
            bytes.append(static_cast<char>((reg >> 8) & 0xFF)); // Старший байт
            bytes.append(static_cast<char>(reg & 0xFF));        // Младший байт
        } else { // LittleEndian
            bytes.append(static_cast<char>(reg & 0xFF));        // Младший байт
            bytes.append(static_cast<char>((reg >> 8) & 0xFF)); // Старший байт
        }
    }

    // Обрабатываем разные форматы строк
    switch (m_format) {
    case NullTerminated:
        // Обрезаем по первому нулевому байту
        bytes.truncate(bytes.indexOf('\0'));
        break;

    case FixedLength:
        // Берем фиксированное количество байт, соответствующее длине строки
        if (m_encoding == "UTF-16" || m_encoding == "UTF-16LE" || m_encoding == "UTF-16BE") {
            byteCount = m_stringLength * 2; // UTF-16: 2 байта на символ
        } else {
            byteCount = m_stringLength;     // Однобайтовые кодировки
        }
        if (bytes.size() > byteCount) {
            bytes.resize(byteCount);
        }
        break;

    case LengthPrefixed:
        // Первый регистр содержит длину строки
        if (data.size() >= 1) {
            int length = data[0];
            int maxBytes = (data.size() - 1) * 2;
            if (length * 2 > maxBytes) {
                length = maxBytes / 2;
            }
            bytes = bytes.mid(2, length * 2); // Пропускаем первый регистр (2 байта)
        }
        break;
    }

    return bytesToString(bytes);
}

QVector<quint16> LineEditBinding::encodeString(const QString& str) const
{
    QByteArray bytes = stringToBytes(str);
    QVector<quint16> result;

    // Обрабатываем разные форматы строк
    switch (m_format) {
    case NullTerminated:
        // Добавляем нулевой терминатор
        bytes.append('\0');
        // Дополняем до четного количества байт
        if (bytes.size() % 2 != 0) {
            bytes.append('\0');
        }
        break;

    case FixedLength:
        // Дополняем/обрезаем до фиксированной длины
        if (m_encoding == "UTF-16" || m_encoding == "UTF-16LE" || m_encoding == "UTF-16BE") {
            bytes.resize(m_stringLength * 2);
        } else {
            bytes.resize(m_stringLength);
        }
        break;

    case LengthPrefixed: {
        // Добавляем префикс длины
        int lengthInRegisters = (bytes.size() + 1) / 2; // Округляем вверх
        Q_UNUSED(lengthInRegisters);
        QByteArray prefixedBytes;

        // Добавляем длину (в символах) как первый регистр
        quint16 lengthRegister = static_cast<quint16>(str.length());
        if (modbus_format() == BigEndian) {
            prefixedBytes.append(static_cast<char>((lengthRegister >> 8) & 0xFF));
            prefixedBytes.append(static_cast<char>(lengthRegister & 0xFF));
        } else {
            prefixedBytes.append(static_cast<char>(lengthRegister & 0xFF));
            prefixedBytes.append(static_cast<char>((lengthRegister >> 8) & 0xFF));
        }

        prefixedBytes.append(bytes);
        bytes = prefixedBytes;

        // Дополняем до четного количества байт
        if (bytes.size() % 2 != 0) {
            bytes.append('\0');
        }
        break;
    }
    }

    // Преобразуем байты в регистры
    for (int i = 0; i < bytes.size(); i += 2) {
        quint16 reg = 0;
        if (i < bytes.size()) {
            quint8 highByte = static_cast<quint8>(bytes[i]);
            quint8 lowByte = (i + 1 < bytes.size()) ? static_cast<quint8>(bytes[i + 1]) : 0;

            if (modbus_format() == BigEndian) {
                reg = (static_cast<quint16>(highByte) << 8) | lowByte;
            } else {
                reg = (static_cast<quint16>(lowByte) << 8) | highByte;
            }
        }
        result.append(reg);
    }

    return result;
}

QByteArray LineEditBinding::stringToBytes(const QString& str) const
{
    QTextCodec* codec = QTextCodec::codecForName(m_encoding.toLatin1());
    if (!codec) {
        // По умолчанию используем Latin1, если указанная кодировка не найдена
        codec = QTextCodec::codecForName("ISO-8859-1");
    }

    if (!codec) {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Modbus, QString("LineEditBinding: не найден кодек для %1").arg(m_encoding));

        return str.toLatin1();
    }

    return codec->fromUnicode(str);
}

QString LineEditBinding::bytesToString(const QByteArray& bytes) const
{
    QTextCodec* codec = QTextCodec::codecForName(m_encoding.toLatin1());
    if (!codec) {
        codec = QTextCodec::codecForName("ISO-8859-1");
    }

    if (!codec) {
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Modbus, QString("LineEditBinding: не найден кодек для %1").arg(m_encoding));
        return QString::fromLatin1(bytes);
    }

    return codec->toUnicode(bytes);
}
