#ifndef LINEEDITBINDING_H
#define LINEEDITBINDING_H

#include <QLineEdit>
#include <QModbusDataUnit>
#include <QObject>
#include <QVector>

#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT LineEditBinding : public RegisterBinding {
    Q_OBJECT
public:
    enum StringFormat {
        NullTerminated,  // C-строка (нуль-терминированная)
        FixedLength,     // Фиксированная длина
        LengthPrefixed   // С префиксом длины (первый регистр - длина)
    };

    static StringFormat stringFormatFromText(const QString& format) {
        QString fmt = format.toLower();
        if (fmt == "fixed") return FixedLength;
        if (fmt == "lengthprefixed" || fmt == "prefixed") return LengthPrefixed;
        return NullTerminated; // по умолчанию
    }

    LineEditBinding(QLineEdit* lineEdit,
                    const QString& registerName,
                    int stringLength = 16,  // длина в символах (для FixedLength и NullTerminated)
                    StringFormat format = NullTerminated,
                    QString encoding = "ASCII",  // ASCII, Latin1, UTF-8, UTF-16
                    BindingDirection direction = ReadWrite,
                    ModbusFormat modbus_format = BigEndian,
                    QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    int stringLength() const { return m_stringLength; }
    void setStringLength(int length) { m_stringLength = length; }

    StringFormat stringFormat() const { return m_format; }
    void setStringFormat(StringFormat format) { m_format = format; }

    QString encoding() const { return m_encoding; }
    void setEncoding(const QString& encoding) { m_encoding = encoding; }

private slots:
    void onTextChanged(const QString& text);
    void onEditingFinished();

private:
    QString decodeString(const QVector<quint16>& data) const;
    QVector<quint16> encodeString(const QString& str) const;
    QByteArray stringToBytes(const QString& str) const;
    QString bytesToString(const QByteArray& bytes) const;

private:
    QLineEdit* m_lineEdit;
    int m_stringLength;      // Длина строки в символах
    StringFormat m_format;   // Формат хранения строки
    QString m_encoding;      // Кодировка строки
    bool m_textChanged;      // Флаг изменения текста
};

#endif // LINEEDITBINDING_H
