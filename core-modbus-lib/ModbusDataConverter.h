#ifndef MODBUSDATACONVERTER_H
#define MODBUSDATACONVERTER_H

#include <QVector>
#include <QVariant>
#include <QString>

#include "core-modbus-lib-global.h"
#include "ModbusBase.h"

CORE_MODBUS_LIB_EXPORT void ModbusDataConverterTest();

class CORE_MODBUS_LIB_EXPORT ModbusDataConverter {
public:
    // Преобразует сырые данные регистров в значение нужного типа
    static QVariant registersToVariant(const QVector<quint16>& data,
                                       const QString& regName,
                                       const QString& dataType,
                                       BytesFormat format = BigEndian);

    // Преобразует значение в сырые данные регистров
    static QVector<quint16> variantToRegisters(const QVariant& value,
                                               const QString& regName,
                                               const QString& dataType,
                                               BytesFormat format = BigEndian,
                                               int expectedSize = 0); // expectedSize для контроля размера (если нужно)

    // Утилиты для конкретных типов (могут использоваться внутри)
    static float toFloat(const QVector<quint16>& data, BytesFormat format);
    static double toDouble(const QVector<quint16>& data, BytesFormat format);
    static quint32 toUInt32(const QVector<quint16>& data, BytesFormat format);
    static qint32 toInt32(const QVector<quint16>& data, BytesFormat format);
    static quint64 toUInt64(const QVector<quint16>& data, BytesFormat format);
    static qint64 toInt64(const QVector<quint16>& data, BytesFormat format);
    static QString toString(const QVector<quint16>& data); // предполагаем ASCII/UTF-8 в регистрах
    static bool toBool(const QVector<quint16>& data, int bit = 0); // из одного регистра, можно указать бит

    static QVector<quint16> fromFloat(float value, BytesFormat format);
    static QVector<quint16> fromDouble(double value, BytesFormat format);
    static QVector<quint16> fromUInt32(quint32 value, BytesFormat format);
    static QVector<quint16> fromInt32(qint32 value, BytesFormat format);
    static QVector<quint16> fromUInt64(quint64 value, BytesFormat format);
    static QVector<quint16> fromInt64(qint64 value, BytesFormat format);
    static QVector<quint16> fromString(const QString& value, int maxRegisters = 0); // если maxRegisters > 0, обрезать
    static QVector<quint16> fromBool(bool value, int bit = 0); // возвращает один регистр с установленным битом
};

#endif // MODBUSDATACONVERTER_H
