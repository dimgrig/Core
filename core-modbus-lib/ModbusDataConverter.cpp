#include "ModbusDataConverter.h"
#include "modbus_functions.h"
#include <QByteArray>
#include <QDebug>

#define ENABLE_CONVERTION

QVariant ModbusDataConverter::registersToVariant(const QVector<quint16>& data,
                                                 const QString& regName,
                                                 const QString& dataType,
                                                 BytesFormat format)
{
    QVariant value = QVariant();
    if (data.isEmpty())
        return value;

    QString type = dataType.toLower();

    if (type.contains("float")) {
        value = QVariant(toFloat(data, format));
    } else if (type.contains("double")) {
        value = QVariant(toDouble(data, format));
    } else if (type.contains("ulong")) {
        value = QVariant(toUInt64(data, format));
    } else if (type.contains("long")) {
        value = QVariant(toInt64(data, format));
    } else if (type.contains("uint")) {
        value = QVariant(toUInt32(data, format));
    } else if (type.contains("int")) {
        value = QVariant(toInt32(data, format));
    } else if (type.contains("ushort")) {
        if (data.size() >= 1)
            value = QVariant(data[0]);
    } else if (type.contains("short")) {
        if (data.size() >= 1)
            value = QVariant(qint16(data[0]));
    } else if (type.contains("bool")) {
        value = QVariant(toBool(data)); // по умолчанию бит 0
    } else if (type.contains("string")) {
        value = QVariant(toString(data));
    } else {
        // неизвестный тип – вернуть как есть
        if (data.size() == 1)
            value = QVariant(data[0]);
        else
            value = QVariant::fromValue(data);
    }

#ifdef ENABLE_CONVERTION
    if (regName.contains("* 0.1")) {
        value = value.toDouble()*0.1;
    } else if (regName.contains("* 0.01")) {
        value = value.toDouble()*0.01;
    }
#endif

    return value;
}

QVector<quint16> ModbusDataConverter::variantToRegisters(const QVariant& value,
                                                         const QString& regName,
                                                         const QString& dataType,
                                                         BytesFormat format,
                                                         int expectedSize)
{
#ifdef ENABLE_CONVERTION
    QVariant _value = value;
    if (regName.contains("* 0.1")) {
        _value = value.toDouble() / 0.1;
    } else if (regName.contains("* 0.01")) {
        _value = value.toDouble() / 0.01;
    }
#endif

    QString type = dataType.toLower();
    QVector<quint16> result;

    if (type.contains("float")) {
        result = fromFloat(_value.toFloat(), format);
    } else if (type.contains("double")) {
        result = fromDouble(_value.toDouble(), format);
    } else if (type.contains("ulong")) {
        result = fromUInt64(_value.toULongLong(), format);
    } else if (type.contains("long")) {
        result = fromInt64(_value.toLongLong(), format);
    } else if (type.contains("uint")) {
        result = fromUInt32(_value.toUInt(), format);
    } else if (type.contains("int")) {
        result = fromInt32(_value.toInt(), format);
    } else if (type.contains("ushort")) {
        result = { static_cast<quint16>(_value.toUInt()) };
    } else if (type.contains("short")) {
        result = { static_cast<quint16>(_value.toInt()) };
    } else if (type.contains("bool")) {
        result = fromBool(_value.toBool());
    } else if (type.contains("string")) {
        result = fromString(_value.toString(), expectedSize);
    } else {
        // fallback: если variant содержит QVector<quint16> напрямую
        if (_value.canConvert<QVector<quint16>>()) {
            result = _value.value<QVector<quint16>>();
        } else {
            qWarning() << "ModbusDataConverter: unknown type" << dataType;
        }
    }

    // При необходимости обрезать/дополнить до ожидаемого размера
    if (expectedSize > 0 && result.size() != expectedSize) {
        if (result.size() < expectedSize) {
            result.resize(expectedSize);
        } else {
            result = result.mid(0, expectedSize);
        }
    }

    return result;
}

// Реализации специфических методов, использующих modbus_functions
float ModbusDataConverter::toFloat(const QVector<quint16>& data, BytesFormat format) {
    if (data.size() < 2) return 0.0f;
    return usa_to_float(data.data(), format);
}

double ModbusDataConverter::toDouble(const QVector<quint16>& data, BytesFormat format) {
    if (data.size() < 4) return 0.0;
    return usa_to_double(data.data(), format);
}

quint32 ModbusDataConverter::toUInt32(const QVector<quint16>& data, BytesFormat format) {
    if (data.size() < 2) return 0;
    return usa_to_uint(data.data(), format);
}

qint32 ModbusDataConverter::toInt32(const QVector<quint16>& data, BytesFormat format) {
    if (data.size() < 2) return 0;
    return usa_to_int(data.data(), format);
}

quint64 ModbusDataConverter::toUInt64(const QVector<quint16>& data, BytesFormat format) {
    if (data.size() < 4) return 0;
    return usa_to_ulong(data.data(), format);
}

qint64 ModbusDataConverter::toInt64(const QVector<quint16>& data, BytesFormat format) {
    if (data.size() < 4) return 0;
    return usa_to_long(data.data(), format);
}

QString ModbusDataConverter::toString(const QVector<quint16>& data) {
    QByteArray bytes;
    for (quint16 reg : data) {
        bytes.append((reg >> 8) & 0xFF);
        bytes.append(reg & 0xFF);
    }
    int nullPos = bytes.indexOf('\0');
    if (nullPos != -1)
        bytes.truncate(nullPos);
    return QString::fromUtf8(bytes);
}

bool ModbusDataConverter::toBool(const QVector<quint16>& data, int bit) {
    if (data.isEmpty()) return false;
    return (data[0] >> bit) & 1;
}

QVector<quint16> ModbusDataConverter::fromFloat(float value, BytesFormat format) {
    QVector<quint16> res(2);
    float_to_usa(value, res.data(), format);
    return res;
}

QVector<quint16> ModbusDataConverter::fromDouble(double value, BytesFormat format) {
    QVector<quint16> res(4);
    double_to_usa(value, res.data(), format);
    return res;
}

QVector<quint16> ModbusDataConverter::fromUInt32(quint32 value, BytesFormat format) {
    QVector<quint16> res(2);
    uint_to_usa(value, res.data(), format);
    return res;
}

QVector<quint16> ModbusDataConverter::fromInt32(qint32 value, BytesFormat format) {
    QVector<quint16> res(2);
    int_to_usa(value, res.data(), format);
    return res;
}

QVector<quint16> ModbusDataConverter::fromUInt64(quint64 value, BytesFormat format) {
    QVector<quint16> res(4);
    ulong_to_usa(value, res.data(), format);
    return res;
}

QVector<quint16> ModbusDataConverter::fromInt64(qint64 value, BytesFormat format) {
    QVector<quint16> res(4);
    long_to_usa(value, res.data(), format);
    return res;
}

QVector<quint16> ModbusDataConverter::fromString(const QString& value, int maxRegisters) {
    QByteArray bytes = value.toUtf8();
    // Добавляем нулевой терминатор, если есть место
    bytes.append('\0');
    int byteCount = bytes.size();
    int regCount = (byteCount + 1) / 2; // округление вверх
    if (maxRegisters > 0 && regCount > maxRegisters) {
        regCount = maxRegisters;
        byteCount = maxRegisters * 2;
        bytes.resize(byteCount);
        if (byteCount > 0) bytes[byteCount-1] = '\0'; // гарантировать терминатор
    }
    QVector<quint16> res(regCount, 0);
    for (int i = 0; i < byteCount; i++) {
        int idx = i / 2;
        if (i % 2 == 0)
            res[idx] = (static_cast<quint8>(bytes[i]) << 8);
        else
            res[idx] |= static_cast<quint8>(bytes[i]);
    }
    return res;
}

QVector<quint16> ModbusDataConverter::fromBool(bool value, int bit) {
    QVector<quint16> res = {0};
    if (value)
        res[0] |= (1 << bit);
    return res;
}



void testFloatConversion() {
    float original = 3.1415f;
    QVector<quint16> regs = ModbusDataConverter::fromFloat(original, BigEndian);
    float result = ModbusDataConverter::toFloat(regs, BigEndian);
    ASSERT(std::abs(result - original) < 1e-5);
}

void testDoubleConversion() {
    double original = 123456.789;
    QVector<quint16> regs = ModbusDataConverter::fromDouble(original, BigEndian);
    double result = ModbusDataConverter::toDouble(regs, BigEndian);
    ASSERT(std::abs(result - original) < 1e-10);
}

void testUInt32Conversion() {
    quint32 original = 0x12345678;
    QVector<quint16> regs = ModbusDataConverter::fromUInt32(original, BigEndian);
    quint32 result = ModbusDataConverter::toUInt32(regs, BigEndian);
    ASSERT_EQUAL(result, original);
}

void testInt32Conversion() {
    qint32 original = -12345678;
    QVector<quint16> regs = ModbusDataConverter::fromInt32(original, BigEndian);
    qint32 result = ModbusDataConverter::toInt32(regs, BigEndian);
    ASSERT_EQUAL(result, original);
}

void testUInt64Conversion() {
    quint64 original = 0x123456789ABCDEF0ULL;
    QVector<quint16> regs = ModbusDataConverter::fromUInt64(original, BigEndian);
    quint64 result = ModbusDataConverter::toUInt64(regs, BigEndian);
    ASSERT_EQUAL(result, original);
}

void testInt64Conversion() {
    qint64 original = -123456789012345LL;
    QVector<quint16> regs = ModbusDataConverter::fromInt64(original, BigEndian);
    qint64 result = ModbusDataConverter::toInt64(regs, BigEndian);
    ASSERT_EQUAL(result, original);
}

void testStringConversion() {
    QString original = "Hello Modbus";
    QVector<quint16> regs = ModbusDataConverter::fromString(original);
    QString result = ModbusDataConverter::toString(regs);
    ASSERT_EQUAL(result, original);
}

void testStringTruncation() {
    QString original = "Too long string";
    int maxRegs = 3; // 6 байт
    QVector<quint16> regs = ModbusDataConverter::fromString(original, maxRegs);
    QString result = ModbusDataConverter::toString(regs);
    ASSERT_EQUAL(result.size(), 5); // строка должна обрезаться до 5 байт + терминатор
}

void testBoolConversion() {
    bool original = true;
    QVector<quint16> regs = ModbusDataConverter::fromBool(original, 2);
    bool result = ModbusDataConverter::toBool(regs, 2);
    ASSERT_EQUAL(result, original);

    original = false;
    regs = ModbusDataConverter::fromBool(original, 2);
    result = ModbusDataConverter::toBool(regs, 2);
    ASSERT_EQUAL(result, original);
}

void testVariantRoundtrip() {
    // Проверка универсального метода
    QVariant original(3.1415f);
    QVector<quint16> regs = ModbusDataConverter::variantToRegisters(original, "name", "float", BigEndian);
    QVariant result = ModbusDataConverter::registersToVariant(regs, "name", "float", BigEndian);
    ASSERT(std::abs(result.toFloat() - original.toFloat()) < 1e-5);

    original = QVariant(123456);
    regs = ModbusDataConverter::variantToRegisters(original, "name", "int", BigEndian);
    result = ModbusDataConverter::registersToVariant(regs, "name", "int", BigEndian);
    ASSERT_EQUAL(result.toInt(), original.toInt());

    original = QVariant("Test");
    regs = ModbusDataConverter::variantToRegisters(original, "name", "string");
    result = ModbusDataConverter::registersToVariant(regs, "name", "string");
    ASSERT_EQUAL(result.toString(), original.toString());
}

void ModbusDataConverterTest() {
    testFloatConversion();
    testDoubleConversion();
    testUInt32Conversion();
    testInt32Conversion();
    testUInt64Conversion();
    testInt64Conversion();
    testStringConversion();
    testStringTruncation();
    testBoolConversion();
    testVariantRoundtrip();
}
