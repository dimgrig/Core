#ifndef MODBUSCSVIMPORTER_H
#define MODBUSCSVIMPORTER_H

#include <QString>
#include <QVector>

#include "core-modbus-lib-global.h"

struct ModbusRegisterMap;

// Одна строка CSV-файла с описанием регистра.
// Формат заголовка (порядок любой, все колонки обязательны):
//   Name;Region;Address;DataTypeDevice;DataTypeScada;Access;
//   TypeString;LengthString;Scale;Comment
struct CORE_MODBUS_LIB_EXPORT ModbusCsvEntry {
    QString name;
    QString region;          // COILS, DISCRETE_INPUTS, INPUT_REGISTERS, HOLDING_REGISTERS
    int     address = 0;
    QString dataTypeDevice;  // BOOL, WORD, INT, DWORD, DINT, REAL, LREAL, LWORD, LINT, STRING
    QString dataTypeScada;
    QString access;          // ReadOnly / ReadWrite
    QString typeString;      // ASCII, UTF-8, ...
    int     lengthString = 0;
    double  scale = 0.0;
    QString comment;
};

CORE_MODBUS_LIB_EXPORT void ModbusCsvImporterTest();

class CORE_MODBUS_LIB_EXPORT ModbusCsvImporter {
public:
    // Разбирает CSV-файл (разделитель ';', все поля в кавычках).
    // При ошибке возвращает пустой список, *ok=false, *error — текст ошибки.
    static QVector<ModbusCsvEntry> parse(const QString& path,
                                         bool* ok = nullptr,
                                         QString* error = nullptr);

    // Преобразует строки CSV в ModbusRegisterMap (mdum/types/access/scales).
    // Записи с невалидным Region/DataType/Address — пропускаются.
    static ModbusRegisterMap toRegisterMap(int serverAddress,
                                           const QVector<ModbusCsvEntry>& entries,
                                           QStringList* warnings = nullptr);

    // Удобная обёртка: parse + toRegisterMap.
    static ModbusRegisterMap loadFile(int serverAddress,
                                      const QString& path,
                                      bool* ok = nullptr,
                                      QString* error = nullptr,
                                      QStringList* warnings = nullptr);
};

#endif // MODBUSCSVIMPORTER_H
