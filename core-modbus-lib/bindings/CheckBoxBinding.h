#ifndef CHECKBOXBINDING_H
#define CHECKBOXBINDING_H

#include <QCheckBox>
#include <QObject>

#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT CheckBoxBinding : public RegisterBinding {
    Q_OBJECT
public:
    CheckBoxBinding(QCheckBox* checkBox,
                    const QString& registerName,
                    int bitPosition = 0,
                    BindingDirection direction = ReadWrite,
                    BytesFormat modbus_format = BigEndian,
                    QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    // Новый метод для работы с общими регистрами
    QVector<quint16> getRegisterDataWithMask(quint16 currentValue) const;

    int bitPosition() const;

private slots:
    void onStateChanged(int state);

private:
    QCheckBox* m_checkBox;
    int m_bitPosition;
};

#endif // CHECKBOXBINDING_H
