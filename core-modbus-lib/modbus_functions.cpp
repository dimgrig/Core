#include "modbus_functions.h"

#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

// ========== Вспомогательные функции ==========

// Определение порядка байтов системы
bool is_system_little_endian() {
    uint16_t test = 0x0001;
    return *reinterpret_cast<uint8_t*>(&test) == 0x01;
}

// Получение байтов из значения
template<typename T>
void get_bytes(T value, uint8_t* bytes) {
    memcpy(bytes, &value, sizeof(T));
}

// Сборка значения из байтов
template<typename T>
T make_value(const uint8_t* bytes) {
    T value;
    memcpy(&value, bytes, sizeof(T));
    return value;
}

// Преобразование байтов между форматами для 32-битных значений
void convert_32bit_format(const uint8_t* src, uint8_t* dst, ModbusFormat from, ModbusFormat to) {
    if (from == to) {
        memcpy(dst, src, 4);
        return;
    }

    // Представим байты в формате from как A B C D
    uint8_t A, B, C, D;

    switch (from) {
    case BigEndian: // A B C D
        A = src[0]; B = src[1]; C = src[2]; D = src[3];
        break;
    case BigEndianByteSwap: // B A D C
        B = src[0]; A = src[1]; D = src[2]; C = src[3];
        break;
    case LittleEndianByteSwap: // C D A B
        C = src[0]; D = src[1]; A = src[2]; B = src[3];
        break;
    case LittleEndian: // D C B A
        D = src[0]; C = src[1]; B = src[2]; A = src[3];
        break;
    }

    // Преобразуем в формат to
    switch (to) {
    case BigEndian: // A B C D
        dst[0] = A; dst[1] = B; dst[2] = C; dst[3] = D;
        break;
    case BigEndianByteSwap: // B A D C
        dst[0] = B; dst[1] = A; dst[2] = D; dst[3] = C;
        break;
    case LittleEndianByteSwap: // C D A B
        dst[0] = C; dst[1] = D; dst[2] = A; dst[3] = B;
        break;
    case LittleEndian: // D C B A
        dst[0] = D; dst[1] = C; dst[2] = B; dst[3] = A;
        break;
    }
}

// Преобразование байтов между форматами для 64-битных значений
void convert_64bit_format(const uint8_t* src, uint8_t* dst, ModbusFormat from, ModbusFormat to) {
    if (from == to) {
        memcpy(dst, src, 8);
        return;
    }

    // Для 64-битных значений рассматриваем пары 32-битных слов
    convert_32bit_format(src, dst, from, to);     // Первые 4 байта
    convert_32bit_format(src + 4, dst + 4, from, to); // Вторые 4 байта
}

// ========== Инверсия порядка слов (без изменения байтового порядка) ==========

unsigned int invert_uint_order(unsigned int value) {
    uint16_t *p = reinterpret_cast<uint16_t*>(&value);
    uint16_t ar[2] = {0, 0};
    ar[0] = *(p + 1);
    ar[1] = *(p + 0);
    return *reinterpret_cast<unsigned int*>(ar);
}

int invert_int_order(int value) {
    uint16_t *p = reinterpret_cast<uint16_t*>(&value);
    uint16_t ar[2] = {0, 0};
    ar[0] = *(p + 1);
    ar[1] = *(p + 0);
    return *reinterpret_cast<int*>(ar);
}

unsigned long long invert_ulong_order(unsigned long long value) {
    uint16_t *p = reinterpret_cast<uint16_t*>(&value);
    uint16_t ar[4] = {0, 0, 0, 0};
    ar[0] = *(p + 3);
    ar[1] = *(p + 2);
    ar[2] = *(p + 1);
    ar[3] = *(p + 0);
    return *reinterpret_cast<unsigned long long*>(ar);
}

long long invert_long_order(long long value) {
    uint16_t *p = reinterpret_cast<uint16_t*>(&value);
    uint16_t ar[4] = {0, 0, 0, 0};
    ar[0] = *(p + 3);
    ar[1] = *(p + 2);
    ar[2] = *(p + 1);
    ar[3] = *(p + 0);
    return *reinterpret_cast<long long*>(ar);
}

float invert_float_order(float value) {
    uint16_t *p = reinterpret_cast<uint16_t*>(&value);
    uint16_t ar[2] = {0, 0};
    ar[0] = *(p + 1);
    ar[1] = *(p + 0);
    return *reinterpret_cast<float*>(ar);
}

double invert_double_order(double value) {
    uint16_t *p = reinterpret_cast<uint16_t*>(&value);
    uint16_t ar[4] = {0, 0, 0, 0};
    ar[0] = *(p + 3);
    ar[1] = *(p + 2);
    ar[2] = *(p + 1);
    ar[3] = *(p + 0);
    return *reinterpret_cast<double*>(ar);
}

// ========== Преобразование значений в регистры Modbus ==========

void uint_to_usa(unsigned int value, unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[4];
    uint8_t dst_bytes[4];

    get_bytes(value, src_bytes);

    // Преобразуем из системного формата в целевой формат
    if (is_system_little_endian()) {
        convert_32bit_format(src_bytes, dst_bytes, LittleEndian, format);
    } else {
        convert_32bit_format(src_bytes, dst_bytes, BigEndian, format);
    }

    // Записываем результат в регистры
    data[0] = (dst_bytes[0] << 8) | dst_bytes[1];
    data[1] = (dst_bytes[2] << 8) | dst_bytes[3];
}

void int_to_usa(int value, unsigned short *data, ModbusFormat format) {
    uint_to_usa(static_cast<unsigned int>(value), data, format);
}

void ulong_to_usa(unsigned long long value, unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[8];
    uint8_t dst_bytes[8];

    get_bytes(value, src_bytes);

    // Преобразуем из системного формата в целевой формат
    if (is_system_little_endian()) {
        convert_64bit_format(src_bytes, dst_bytes, LittleEndian, format);
    } else {
        convert_64bit_format(src_bytes, dst_bytes, BigEndian, format);
    }

    // Записываем результат в регистры
    for (int i = 0; i < 4; ++i) {
        data[i] = (dst_bytes[i * 2] << 8) | dst_bytes[i * 2 + 1];
    }
}

void long_to_usa(long long value, unsigned short *data, ModbusFormat format) {
    ulong_to_usa(static_cast<unsigned long long>(value), data, format);
}

void float_to_usa(float value, unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[4];
    uint8_t dst_bytes[4];

    get_bytes(value, src_bytes);

    // Преобразуем из системного формата в целевой формат
    if (is_system_little_endian()) {
        convert_32bit_format(src_bytes, dst_bytes, LittleEndian, format);
    } else {
        convert_32bit_format(src_bytes, dst_bytes, BigEndian, format);
    }

    // Записываем результат в регистры
    data[0] = (dst_bytes[0] << 8) | dst_bytes[1];
    data[1] = (dst_bytes[2] << 8) | dst_bytes[3];
}

void double_to_usa(double value, unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[8];
    uint8_t dst_bytes[8];

    get_bytes(value, src_bytes);

    // Преобразуем из системного формата в целевой формат
    if (is_system_little_endian()) {
        convert_64bit_format(src_bytes, dst_bytes, LittleEndian, format);
    } else {
        convert_64bit_format(src_bytes, dst_bytes, BigEndian, format);
    }

    // Записываем результат в регистры
    for (int i = 0; i < 4; ++i) {
        data[i] = (dst_bytes[i * 2] << 8) | dst_bytes[i * 2 + 1];
    }
}

// ========== Преобразование регистров Modbus в значения ==========

unsigned int usa_to_uint(const unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[4];
    uint8_t dst_bytes[4];

    // Читаем байты из регистров
    src_bytes[0] = (data[0] >> 8) & 0xFF;
    src_bytes[1] = data[0] & 0xFF;
    src_bytes[2] = (data[1] >> 8) & 0xFF;
    src_bytes[3] = data[1] & 0xFF;

    // Преобразуем из целевого формата в системный формат
    if (is_system_little_endian()) {
        convert_32bit_format(src_bytes, dst_bytes, format, LittleEndian);
    } else {
        convert_32bit_format(src_bytes, dst_bytes, format, BigEndian);
    }

    return make_value<unsigned int>(dst_bytes);
}

int usa_to_int(const unsigned short *data, ModbusFormat format) {
    return static_cast<int>(usa_to_uint(data, format));
}

unsigned long long usa_to_ulong(const unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[8];
    uint8_t dst_bytes[8];

    // Читаем байты из регистров
    for (int i = 0; i < 4; ++i) {
        src_bytes[i * 2] = (data[i] >> 8) & 0xFF;
        src_bytes[i * 2 + 1] = data[i] & 0xFF;
    }

    // Преобразуем из целевого формата в системный формат
    if (is_system_little_endian()) {
        convert_64bit_format(src_bytes, dst_bytes, format, LittleEndian);
    } else {
        convert_64bit_format(src_bytes, dst_bytes, format, BigEndian);
    }

    return make_value<unsigned long long>(dst_bytes);
}

long long usa_to_long(const unsigned short *data, ModbusFormat format) {
    return static_cast<long long>(usa_to_ulong(data, format));
}

float usa_to_float(const unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[4];
    uint8_t dst_bytes[4];

    // Читаем байты из регистров
    src_bytes[0] = (data[0] >> 8) & 0xFF;
    src_bytes[1] = data[0] & 0xFF;
    src_bytes[2] = (data[1] >> 8) & 0xFF;
    src_bytes[3] = data[1] & 0xFF;

    // Преобразуем из целевого формата в системный формат
    if (is_system_little_endian()) {
        convert_32bit_format(src_bytes, dst_bytes, format, LittleEndian);
    } else {
        convert_32bit_format(src_bytes, dst_bytes, format, BigEndian);
    }

    return make_value<float>(dst_bytes);
}

double usa_to_double(const unsigned short *data, ModbusFormat format) {
    uint8_t src_bytes[8];
    uint8_t dst_bytes[8];

    // Читаем байты из регистров
    for (int i = 0; i < 4; ++i) {
        src_bytes[i * 2] = (data[i] >> 8) & 0xFF;
        src_bytes[i * 2 + 1] = data[i] & 0xFF;
    }

    // Преобразуем из целевого формата в системный формат
    if (is_system_little_endian()) {
        convert_64bit_format(src_bytes, dst_bytes, format, LittleEndian);
    } else {
        convert_64bit_format(src_bytes, dst_bytes, format, BigEndian);
    }

    return make_value<double>(dst_bytes);
}

// ========== Конвертация между форматами ==========

template<typename T>
T convert_modbus_format(T value, ModbusFormat from_format, ModbusFormat to_format) {
    if (from_format == to_format) {
        return value;
    }

    uint8_t src_bytes[sizeof(T)];
    uint8_t dst_bytes[sizeof(T)];

    get_bytes(value, src_bytes);

    if constexpr (sizeof(T) == 4) {
        convert_32bit_format(src_bytes, dst_bytes, from_format, to_format);
    } else if constexpr (sizeof(T) == 8) {
        convert_64bit_format(src_bytes, dst_bytes, from_format, to_format);
    } else {
        // Для других размеров просто копируем
        memcpy(dst_bytes, src_bytes, sizeof(T));
    }

    return make_value<T>(dst_bytes);
}

// Явные инстанциации шаблона для удобства
template float convert_modbus_format<float>(float, ModbusFormat, ModbusFormat);
template double convert_modbus_format<double>(double, ModbusFormat, ModbusFormat);
template unsigned int convert_modbus_format<unsigned int>(unsigned int, ModbusFormat, ModbusFormat);
template int convert_modbus_format<int>(int, ModbusFormat, ModbusFormat);
template unsigned long long convert_modbus_format<unsigned long long>(unsigned long long, ModbusFormat, ModbusFormat);
template long long convert_modbus_format<long long>(long long, ModbusFormat, ModbusFormat);


// Вспомогательная функция для сравнения чисел с плавающей точкой с допуском
template<typename T>
bool approx_equal(T a, T b, T epsilon = std::numeric_limits<T>::epsilon() * 100) {
    return std::fabs(a - b) <= epsilon;
}

// Тест симметричности преобразования для заданного типа и формата
template<typename T>
void testRoundTrip(T originalValue, ModbusFormat format, const std::string& typeName) {
    const size_t regCount = sizeof(T) / sizeof(uint16_t);
    uint16_t registers[regCount];

    // Запись в регистры
    if constexpr (std::is_same_v<T, float>) {
        float_to_usa(originalValue, registers, format);
    } else if constexpr (std::is_same_v<T, double>) {
        double_to_usa(originalValue, registers, format);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        uint_to_usa(originalValue, registers, format);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        int_to_usa(originalValue, registers, format);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        ulong_to_usa(originalValue, registers, format);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        long_to_usa(originalValue, registers, format);
    }

    // Чтение из регистров
    T result;
    if constexpr (std::is_same_v<T, float>) {
        result = usa_to_float(registers, format);
    } else if constexpr (std::is_same_v<T, double>) {
        result = usa_to_double(registers, format);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        result = usa_to_uint(registers, format);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        result = usa_to_int(registers, format);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        result = usa_to_ulong(registers, format);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        result = usa_to_long(registers, format);
    }

    // Проверка
    if constexpr (std::is_floating_point_v<T>) {
        ASSERT(approx_equal(result, originalValue));
    } else {
        ASSERT_EQUAL(result, originalValue);
    }
}

// Тест для всех форматов одного типа
template<typename T>
void testAllFormats(T testValue, const std::string& typeName) {
    std::vector<ModbusFormat> formats = {BigEndian, BigEndianByteSwap, LittleEndianByteSwap, LittleEndian};
    for (auto fmt : formats) {
        testRoundTrip(testValue, fmt, typeName);
    }
}

// ----------------------------------------------------------------------
// Тесты для конкретных типов
// ----------------------------------------------------------------------

void testFloat() {
    float val = 1234.5678f;
    testAllFormats(val, "float");
}

void testDouble() {
    double val = 123456789.123456;
    testAllFormats(val, "double");
}

void testUint32() {
    uint32_t val = 0x12345678;
    testAllFormats(val, "uint32");
}

void testInt32() {
    int32_t val = -12345678;
    testAllFormats(val, "int32");
}

void testUint64() {
    uint64_t val = 0x123456789ABCDEF0ULL;
    testAllFormats(val, "uint64");
}

void testInt64() {
    int64_t val = -123456789012345LL;
    testAllFormats(val, "int64");
}

// Тест для граничных значений
void testEdgeCases() {
    // float: 0, бесконечность, NaN (не поддерживается стандартным сравнением, пропускаем)
    float fZero = 0.0f;
    testRoundTrip(fZero, BigEndian, "float_zero");

    double dZero = 0.0;
    testRoundTrip(dZero, BigEndian, "double_zero");

    // Максимальные значения
    uint32_t maxU32 = std::numeric_limits<uint32_t>::max();
    testRoundTrip(maxU32, BigEndian, "uint32_max");

    int32_t minI32 = std::numeric_limits<int32_t>::min();
    testRoundTrip(minI32, BigEndian, "int32_min");

    uint64_t maxU64 = std::numeric_limits<uint64_t>::max();
    testRoundTrip(maxU64, BigEndian, "uint64_max");
}

// Тест функций инверсии порядка слов
void testInvertOrder() {
    // 32-bit
    uint32_t u32 = 0x12345678;
    uint32_t invU32 = invert_uint_order(u32);
    ASSERT_EQUAL(invU32, 0x56781234U);  // порядок слов меняется местами

    int32_t i32 = -12345678;
    int32_t invI32 = invert_int_order(i32);
    // Проверим симметричность
    ASSERT_EQUAL(invert_int_order(invI32), i32);

    // 64-bit
    uint64_t u64 = 0x123456789ABCDEF0ULL;
    uint64_t invU64 = invert_ulong_order(u64);
    ASSERT_EQUAL(invU64, 0xDEF09ABC56781234ULL);

    double d = 123.456;
    double invD = invert_double_order(d);
    // Проверим симметричность
    ASSERT(approx_equal(invert_double_order(invD), d));
}

void ModbusFunctionsTest() {
    testFloat();
    testDouble();
    testUint32();
    testInt32();
    testUint64();
    testInt64();
    testEdgeCases();
    testInvertOrder();
}
