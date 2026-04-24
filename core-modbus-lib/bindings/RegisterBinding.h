#ifndef REGISTERBINDING_H
#define REGISTERBINDING_H

#include "ModbusBase.h"

#include <QObject>
class CORE_MODBUS_LIB_EXPORT DataConversionUtils {
public:
    // Преобразование float в два 16-битных регистра
    static QVector<quint16> floatToRegisters(float value, bool bigEndian = false);

    // Преобразование двух 16-битных регистров в float
    static float registersToFloat(const QVector<quint16>& registers, bool bigEndian = false);

    // Преобразование double в четыре 16-битных регистра
    static QVector<quint16> doubleToRegisters(double value, bool bigEndian = false);

    // Преобразование четырех 16-битных регистров в double
    static double registersToDouble(const QVector<quint16>& registers, bool bigEndian = false);

    // Преобразование 32-битного целого в два 16-битных регистра
    static QVector<quint16> int32ToRegistersonnx(qint32 value, bool bigEndian = false);

    // Преобразование двух 16-битных регистров в 32-битное целое
    static qint32 registersToInt32(const QVector<quint16>& registers, bool bigEndian = false);

    // Извлечение бита из регистра
    static bool getBit(quint16 registerValue, int bitPosition);

    // Установка бита в регистре
    static quint16 setBit(quint16 registerValue, int bitPosition, bool bitValue);

    // Проверка корректности размера данных для типа
    static bool validateSize(const QVector<quint16>& data, int expectedSize);
};

class CORE_MODBUS_LIB_EXPORT RegisterBinding : public QObject {
    Q_OBJECT

public:
    enum BindingDirection {
        ReadOnly,    // Только чтение из регистра
        WriteOnly,   // Только запись в регистр
        ReadWrite    // Чтение и запись
    };

    explicit RegisterBinding(const QString& registerName,
                             BindingDirection direction = ReadWrite,
                             BytesFormat modbus_format = BigEndian,
                             QObject* parent = nullptr);

    virtual ~RegisterBinding();

    // Чисто виртуальные методы, которые должны быть реализованы в производных классах
    virtual void updateFromRegister(const QVector<quint16>& data) = 0;
    virtual QVector<quint16> getRegisterData() const = 0;
    virtual bool isValid() const = 0;

    // Геттеры
    QString registerName() const { return m_registerName; }
    BindingDirection direction() const { return m_direction; }
    BytesFormat modbus_format() const { return m_modbus_format; }

    // Сеттеры
    void setRegisterName(const QString& name);
    void setDirection(BindingDirection direction);
    void setBytesFormat(BytesFormat modbus_format);

    // Метод для принудительного чтения
    void requestRead();

    // Статические вспомогательные методы
    static QString directionToString(BindingDirection direction);
    static BindingDirection stringToDirection(const QString& directionStr);

signals:
    // Сигнал испускается когда значение изменилось через UI (для записи в регистр)
    void valueChanged();

    // Сигнал для прямого запроса на изменение значения регистра
    void registerValueRequested(const QString& registerName, const QVector<quint16>& data);

    // Сигнал для запроса чтения
    void readRequested(const QString& registerName);

protected:
    QString m_registerName;
    BindingDirection m_direction;
    BytesFormat m_modbus_format;
};

#endif // REGISTERBINDING_H
