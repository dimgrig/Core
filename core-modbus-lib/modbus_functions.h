#ifndef MODBUS_FUNCTIONS_H
#define MODBUS_FUNCTIONS_H

#include "core-modbus-lib-global.h"

#include "ModbusBase.h"

CORE_MODBUS_LIB_EXPORT void ModbusFunctionsTest();

// ========== ОСНОВНЫЕ ФУНКЦИИ ==========

// Инверсия порядка 16-битных слов (без изменения формата)
CORE_MODBUS_LIB_EXPORT unsigned int invert_uint_order(unsigned int value);
CORE_MODBUS_LIB_EXPORT int invert_int_order(int value);
CORE_MODBUS_LIB_EXPORT unsigned long long invert_ulong_order(unsigned long long value);
CORE_MODBUS_LIB_EXPORT long long invert_long_order(long long value);
CORE_MODBUS_LIB_EXPORT float invert_float_order(float value);
CORE_MODBUS_LIB_EXPORT double invert_double_order(double value);

// Преобразование значения в массив 16-битных регистров
CORE_MODBUS_LIB_EXPORT void uint_to_usa(unsigned int value, unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT void int_to_usa(int value, unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT void ulong_to_usa(unsigned long long value, unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT void long_to_usa(long long value, unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT void float_to_usa(float value, unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT void double_to_usa(double value, unsigned short *data, BytesFormat format = BigEndian);

// Преобразование массива 16-битных регистров в значение
CORE_MODBUS_LIB_EXPORT unsigned int usa_to_uint(const unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT int usa_to_int(const unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT unsigned long long usa_to_ulong(const unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT long long usa_to_long(const unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT float usa_to_float(const unsigned short *data, BytesFormat format = BigEndian);
CORE_MODBUS_LIB_EXPORT double usa_to_double(const unsigned short *data, BytesFormat format = BigEndian);

// ========== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ ==========

// Конвертация значения между разными форматами Modbus
template<typename T>
CORE_MODBUS_LIB_EXPORT T convert_modbus_format(T value, BytesFormat from_format, BytesFormat to_format);

// Определение порядка байтов системы
CORE_MODBUS_LIB_EXPORT bool is_system_little_endian();

#endif // MODBUS_FUNCTIONS_H
