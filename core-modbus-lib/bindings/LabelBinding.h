#ifndef LABELBINDING_H
#define LABELBINDING_H

#include <QLabel>
#include <QModbusDataUnit>
#include <QObject>

#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT LabelBinding : public RegisterBinding {
    Q_OBJECT
public:
    enum DisplayFormat {
        Decimal,
        Hexadecimal,
        Binary,
        Float,
        Double,
        Custom,
        BitField
    };

    LabelBinding(QLabel* label,
                 const QString& registerName,
                 DisplayFormat format = Decimal,
                 const QString& customFormat = "%1",
                 int precision = 2,
                 BytesFormat modbus_format = BigEndian,
                 QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    void setFormat(DisplayFormat format);
    void setCustomFormat(const QString& format);
    void setPrecision(int precision);

private:
    QLabel* m_label;
    DisplayFormat m_format;
    QString m_customFormat;
    int m_precision;
};
#endif // LABELBINDING_H
