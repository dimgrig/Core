#ifndef COMBOBOXBINDING_H
#define COMBOBOXBINDING_H

#include <QComboBox>
#include <QMap>
#include <QObject>

#include "RegisterBinding.h"

class CORE_MODBUS_LIB_EXPORT ComboBoxBinding : public RegisterBinding {
    Q_OBJECT
public:
    // Конструктор для прямого соответствия индекса и значения регистра
    ComboBoxBinding(QComboBox* comboBox,
                    const QString& registerName,
                    BindingDirection direction = ReadWrite,
                    ModbusFormat modbus_format = BigEndian,
                    QObject* parent = nullptr);

    // Конструктор с пользовательским отображением значений
    ComboBoxBinding(QComboBox* comboBox,
                    const QString& registerName,
                    const QMap<quint16, int>& valueToIndexMap, // значение регистра -> индекс комбобокса
                    BindingDirection direction = ReadWrite,
                    ModbusFormat modbus_format = BigEndian,
                    QObject* parent = nullptr);

    // Конструктор для работы с битовыми полями
    ComboBoxBinding(QComboBox* comboBox,
                    const QString& registerName,
                    int bitPosition,
                    BindingDirection direction = ReadWrite,
                    ModbusFormat modbus_format = BigEndian,
                    QObject* parent = nullptr);

    void updateFromRegister(const QVector<quint16>& data) override;
    QVector<quint16> getRegisterData() const override;
    bool isValid() const override;

    // Методы для управления отображением значений
    void setValueMap(const QMap<quint16, int>& valueToIndexMap);
    void setBitMode(bool bitMode, int bitPosition = -1);
    void setExpectedSize(int size) { m_expectedSize = size; }

private slots:
    void onCurrentIndexChanged(int index);

private:
    QComboBox* m_comboBox;
    QMap<quint16, int> m_valueToIndexMap;
    bool m_useValueMap;
    bool m_bitMode;
    int m_bitPosition;
    int m_expectedSize;

    quint16 getRegisterValueFromIndex(int index) const;
    int getIndexFromRegisterValue(quint16 value) const;
};

#endif // COMBOBOXBINDING_H
