#include "ModbusCsvImporter.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

#include "ModbusBase.h"
#include "ModbusDataConverter.h"
#include "ModbusRegisterSerializer.h"

// ────────────────────────────────────────────────────────────────
//  Низкоуровневый парсер одной CSV-строки.
//  Разделитель — ';'. Поле в кавычках ("..."). Кавычка внутри поля — "".
// ────────────────────────────────────────────────────────────────
static QStringList parseCsvLine(const QString& line) {
    QStringList fields;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current.append('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                current.append(c);
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ';') {
                fields.append(current);
                current.clear();
            } else {
                current.append(c);
            }
        }
    }
    fields.append(current);
    return fields;
}

// ────────────────────────────────────────────────────────────────
//  Маппинги Region / DataTypeDevice
// ────────────────────────────────────────────────────────────────
struct RegionInfo {
    QModbusDataUnit::RegisterType type;
    bool valid = false;
};

static RegionInfo parseRegion(const QString& r) {
    const QString u = r.trimmed().toUpper();
    if (u == "COILS")             return { QModbusDataUnit::Coils, true };
    if (u == "DISCRETE_INPUTS")   return { QModbusDataUnit::DiscreteInputs, true };
    if (u == "INPUT_REGISTERS")   return { QModbusDataUnit::InputRegisters, true };
    if (u == "HOLDING_REGISTERS") return { QModbusDataUnit::HoldingRegisters, true };
    return { QModbusDataUnit::Invalid, false };
}

// Возвращает пару (внутреннее имя типа для ModbusDataConverter, valueCount).
// valueCount = -1 для STRING — считается отдельно из LengthString.
static std::pair<QString, int> parseDataTypeDevice(const QString& t) {
    const QString s = t.trimmed();

    // ── Промышленные имена IEC 61131-3 (UPPERCASE) ──
    if (s == "BOOL")   return { QStringLiteral("bool"),   1 };
    if (s == "WORD")   return { QStringLiteral("ushort"), 1 };
    if (s == "INT")    return { QStringLiteral("short"),  1 };
    if (s == "DWORD")  return { QStringLiteral("uint"),   2 };
    if (s == "DINT")   return { QStringLiteral("int"),    2 };
    if (s == "LWORD")  return { QStringLiteral("ulong"),  4 };
    if (s == "LINT")   return { QStringLiteral("long"),   4 };
    if (s == "REAL")   return { QStringLiteral("float"),  2 };
    if (s == "LREAL")  return { QStringLiteral("double"), 4 };
    if (s == "STRING") return { QStringLiteral("string"), -1 };

    // ── Внутренние имена ModbusRegisterDataType_to_text ──
    if (s == "Bool")       return { QStringLiteral("bool"),   1 };
    if (s == "UShort")     return { QStringLiteral("ushort"), 1 };
    if (s == "Short")      return { QStringLiteral("short"),  1 };
    if (s == "UInt32")     return { QStringLiteral("uint"),   2 };
    if (s == "Int32")      return { QStringLiteral("int"),    2 };
    if (s == "ULongLong")  return { QStringLiteral("ulong"),  4 };
    if (s == "LongLong")   return { QStringLiteral("long"),   4 };
    if (s == "Float")      return { QStringLiteral("float"),  2 };
    if (s == "Double")     return { QStringLiteral("double"), 4 };
    if (s == "String")     return { QStringLiteral("string"), -1 };
    return { QString(), 0 };
}

// ────────────────────────────────────────────────────────────────
//  parse
// ────────────────────────────────────────────────────────────────
QVector<ModbusCsvEntry> ModbusCsvImporter::parse(const QString& path,
                                                 bool* ok,
                                                 QString* error) {
    if (ok) *ok = false;
    if (error) error->clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = QStringLiteral("Не удалось открыть файл: %1").arg(path);
        return {};
    }

    QTextStream ts(&f);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ts.setCodec("UTF-8");
#endif

    // Первая непустая строка — заголовок.
    QString headerLine;
    while (!ts.atEnd()) {
        headerLine = ts.readLine();
        if (!headerLine.trimmed().isEmpty()) break;
    }
    if (headerLine.trimmed().isEmpty()) {
        if (error) *error = QStringLiteral("Файл пуст: %1").arg(path);
        return {};
    }

    QStringList headerFields = parseCsvLine(headerLine);
    // Нормализуем имена колонок
    for (QString& h : headerFields) h = h.trimmed();
    headerFields[0].remove(QChar(0xFEFF)); // BOM

    auto colIndex = [&](const QString& name) -> int {
        for (int i = 0; i < headerFields.size(); ++i) {
            if (headerFields[i].compare(name, Qt::CaseInsensitive) == 0) return i;
        }
        return -1;
    };

    struct ColMap {
        int name, region, address, dataTypeDevice, dataTypeScada,
            access, typeString, lengthString, scale, comment;
    } col;

    col.name           = colIndex("Name");
    col.region         = colIndex("Region");
    col.address        = colIndex("Address");
    col.dataTypeDevice = colIndex("DataTypeDevice");
    col.dataTypeScada  = colIndex("DataTypeScada");
    col.access         = colIndex("Access");
    col.typeString     = colIndex("TypeString");
    col.lengthString   = colIndex("LengthString");
    col.scale          = colIndex("Scale");
    col.comment        = colIndex("Comment");

    QStringList missingCols;
    if (col.name < 0)           missingCols << "Name";
    if (col.region < 0)         missingCols << "Region";
    if (col.address < 0)        missingCols << "Address";
    if (col.dataTypeDevice < 0) missingCols << "DataTypeDevice";
    if (col.access < 0)         missingCols << "Access";
    if (col.lengthString < 0)   missingCols << "LengthString";
    if (col.scale < 0)          missingCols << "Scale";

    if (!missingCols.isEmpty()) {
        if (error) *error = QStringLiteral("Отсутствуют колонки: %1").arg(missingCols.join(", "));
        return {};
    }

    QVector<ModbusCsvEntry> result;
    int lineNo = 1;

    while (!ts.atEnd()) {
        const QString line = ts.readLine();
        ++lineNo;
        if (line.trimmed().isEmpty()) continue;

        const QStringList fields = parseCsvLine(line);
        auto field = [&](int idx) -> QString {
            return (idx >= 0 && idx < fields.size()) ? fields[idx] : QString();
        };

        ModbusCsvEntry e;
        e.name           = field(col.name).trimmed();
        e.region         = field(col.region).trimmed();
        e.address        = field(col.address).trimmed().toInt();
        e.dataTypeDevice = field(col.dataTypeDevice).trimmed();
        e.dataTypeScada  = field(col.dataTypeScada).trimmed();
        e.access         = field(col.access).trimmed();
        e.typeString     = field(col.typeString).trimmed();
        e.lengthString   = field(col.lengthString).trimmed().toInt();
        e.scale          = field(col.scale).trimmed();
        e.comment        = field(col.comment);

        if (e.name.isEmpty()) continue; // пустая строка
        result.append(e);
    }

    if (ok) *ok = true;
    return result;
}

// ────────────────────────────────────────────────────────────────
//  toRegisterMap
// ────────────────────────────────────────────────────────────────
ModbusRegisterMap ModbusCsvImporter::toRegisterMap(int serverAddress,
                                                   const QVector<ModbusCsvEntry>& entries,
                                                   QStringList* warnings) {
    ModbusRegisterMap map;

    for (const ModbusCsvEntry& e : entries) {
        const RegionInfo rinfo = parseRegion(e.region);
        if (!rinfo.valid) {
            if (warnings) warnings->append(
                    QStringLiteral("%1: неизвестный Region '%2'").arg(e.name, e.region));
            continue;
        }

        const auto [typeStr, valueCountFixed] = parseDataTypeDevice(e.dataTypeDevice);
        if (typeStr.isEmpty()) {
            if (warnings) warnings->append(
                    QStringLiteral("%1: неизвестный DataTypeDevice '%2'").arg(e.name, e.dataTypeDevice));
            continue;
        }

        if (e.address < 0) {
            if (warnings) warnings->append(
                    QStringLiteral("%1: отрицательный Address %2").arg(e.name).arg(e.address));
            continue;
        }

        int valueCount = valueCountFixed;
        if (typeStr == "string") {
            if (e.lengthString <= 0) {
                if (warnings) warnings->append(
                        QStringLiteral("%1: STRING без LengthString").arg(e.name));
                continue;
            }
            valueCount = (e.lengthString + 1) / 2; // байт -> регистров (с округлением вверх)
        }

        if (rinfo.type == QModbusDataUnit::Coils ||
            rinfo.type == QModbusDataUnit::DiscreteInputs) {
            if (typeStr != "bool") {
                if (warnings) warnings->append(
                        QStringLiteral("%1: для %2 допустим только BOOL, получено %3")
                            .arg(e.name, e.region, e.dataTypeDevice));
                continue;
            }
            valueCount = 1;
        }

        QModbusDataUnit unit(rinfo.type, e.address, static_cast<quint16>(valueCount));
        map.mdum[serverAddress][e.name]   = unit;
        map.types[serverAddress][e.name]  = typeStr;
        map.access[serverAddress][e.name] = ModbusRegisterAccess_from_text(e.access.toStdString().c_str());
        if (!e.scale.isEmpty()) {
            ModbusScale sc = ModbusScale::parse(e.scale);
            if (!sc.isNone())
                map.scales[serverAddress][e.name] = sc;
        }
    }

    return map;
}

// ────────────────────────────────────────────────────────────────
//  loadFile
// ────────────────────────────────────────────────────────────────
ModbusRegisterMap ModbusCsvImporter::loadFile(int serverAddress,
                                              const QString& path,
                                              bool* ok,
                                              QString* error,
                                              QStringList* warnings) {
    bool parseOk = false;
    QVector<ModbusCsvEntry> entries = parse(path, &parseOk, error);
    if (!parseOk) {
        if (ok) *ok = false;
        return {};
    }
    ModbusRegisterMap map = toRegisterMap(serverAddress, entries, warnings);
    if (ok) *ok = !map.mdum.empty();
    if (ok && !*ok && error && error->isEmpty())
        *error = QStringLiteral("CSV не содержит валидных регистров");
    return map;
}

// ────────────────────────────────────────────────────────────────
//  Тест
// ────────────────────────────────────────────────────────────────
#include <QTemporaryFile>

static QTemporaryFile* makeCsvFile(const QByteArray& content) {
    auto* f = new QTemporaryFile();
    f->open();
    f->write(content);
    f->close();
    return f;
}

static const QByteArray kMinimalCsv =
    "\"Name\";\"Region\";\"Address\";\"DataTypeDevice\";\"DataTypeScada\";"
    "\"Access\";\"TypeString\";\"LengthString\";\"Scale\";\"Comment\"\n"
    "\"Coil1\";\"COILS\";\"0\";\"BOOL\";\"BOOL\";\"ReadWrite\";\"\";\"\";\"\";\"Coil1\"\n"
    "\"input1\";\"DISCRETE_INPUTS\";\"0\";\"BOOL\";\"BOOL\";\"ReadOnly\";\"\";\"\";\"\";\"input1\"\n"
    "\"ir1\";\"INPUT_REGISTERS\";\"0\";\"WORD\";\"WORD\";\"ReadOnly\";\"\";\"\";\"\";\"ir1\"\n"
    "\"ir5\";\"INPUT_REGISTERS\";\"6\";\"REAL\";\"REAL\";\"ReadOnly\";\"\";\"\";\"\";\"ir5\"\n"
    "\"hr7\";\"HOLDING_REGISTERS\";\"10\";\"LREAL\";\"LREAL\";\"ReadWrite\";\"\";\"\";\"\";\"hr7\"\n"
    "\"InputString\";\"INPUT_REGISTERS\";\"26\";\"STRING\";\"STRING\";\"ReadOnly\";\"ASCII\";\"20\";\"\";\"InputString\"\n"
    "\"hr_scaled\";\"HOLDING_REGISTERS\";\"30\";\"INT\";\"INT\";\"ReadWrite\";\"\";\"\";\"0.1\";\"scaled\"\n";

void ModbusCsvImporterTest() {
    // 1. Минимальный корректный CSV
    {
        QTemporaryFile f;
        f.open();
        f.write(kMinimalCsv);
        f.close();

        bool ok = false;
        QString error;
        auto entries = ModbusCsvImporter::parse(f.fileName(), &ok, &error);
        ASSERT(ok);
        ASSERT(error.isEmpty());
        ASSERT_EQUAL(entries.size(), 7);

        // Проверка полей первой записи
        ASSERT_EQUAL(entries[0].name, QString("Coil1"));
        ASSERT_EQUAL(entries[0].region, QString("COILS"));
        ASSERT_EQUAL(entries[0].address, 0);
        ASSERT_EQUAL(entries[0].dataTypeDevice, QString("BOOL"));
        ASSERT_EQUAL(entries[0].access, QString("ReadWrite"));

        // Проверка строкового регистра
        ASSERT_EQUAL(entries[5].name, QString("InputString"));
        ASSERT_EQUAL(entries[5].typeString, QString("ASCII"));
        ASSERT_EQUAL(entries[5].lengthString, 20);

        // Проверка scale
        ASSERT_EQUAL(entries[6].scale, QString("0.1"));
    }

    // 2. toRegisterMap
    {
        QTemporaryFile f;
        f.open();
        f.write(kMinimalCsv);
        f.close();

        bool ok = false;
        QStringList warnings;
        auto map = ModbusCsvImporter::loadFile(1, f.fileName(), &ok, nullptr, &warnings);

        ASSERT(ok);
        ASSERT(warnings.isEmpty());
        ASSERT_EQUAL(static_cast<int>(map.mdum[1].size()), 7);

        // Типы
        ASSERT_EQUAL(map.types[1]["Coil1"], QString("bool"));
        ASSERT_EQUAL(map.types[1]["ir1"],   QString("ushort"));
        ASSERT_EQUAL(map.types[1]["ir5"],   QString("float"));
        ASSERT_EQUAL(map.types[1]["hr7"],   QString("double"));
        ASSERT_EQUAL(map.types[1]["InputString"], QString("string"));

        // Регистры: типы и valueCount
        ASSERT_EQUAL(map.mdum[1]["Coil1"].registerType(), QModbusDataUnit::Coils);
        ASSERT_EQUAL(map.mdum[1]["Coil1"].valueCount(), 1);

        ASSERT_EQUAL(map.mdum[1]["input1"].registerType(), QModbusDataUnit::DiscreteInputs);
        ASSERT_EQUAL(map.mdum[1]["input1"].valueCount(), 1);

        ASSERT_EQUAL(map.mdum[1]["ir1"].registerType(), QModbusDataUnit::InputRegisters);
        ASSERT_EQUAL(map.mdum[1]["ir1"].valueCount(), 1);

        ASSERT_EQUAL(map.mdum[1]["ir5"].valueCount(), 2);    // REAL
        ASSERT_EQUAL(map.mdum[1]["hr7"].valueCount(), 4);    // LREAL
        ASSERT_EQUAL(map.mdum[1]["InputString"].valueCount(), 10); // 20 байт / 2

        // Access
        ASSERT(map.access[1]["Coil1"] == ModbusRegisterAccess_ReadWrite);
        ASSERT(map.access[1]["input1"] == ModbusRegisterAccess_ReadOnly);

        // Scale
        ASSERT(map.scales[1].count("hr_scaled") == 1);
        ASSERT(map.scales[1]["hr_scaled"].isFactor());
        ASSERT(map.scales[1]["hr_scaled"].factor > 0.099 &&
               map.scales[1]["hr_scaled"].factor < 0.101);
        ASSERT(map.scales[1].count("Coil1") == 0);

        // Round-trip через ModbusDataConverter: CSV -> internal -> converter
        {
            // CSV REAL -> internal "float" -> ModbusDataConverter
            auto regs = ModbusDataConverter::variantToRegisters(QVariant(3.14f), "ir5", map.types[1]["ir5"], BigEndian);
            ASSERT_EQUAL(regs.size(), 2);
            auto back = ModbusDataConverter::registersToVariant(regs, "ir5", map.types[1]["ir5"], BigEndian);
            ASSERT(std::abs(back.toFloat() - 3.14f) < 1e-4);

            // CSV DINT -> internal "int" -> ModbusDataConverter
            auto regs2 = ModbusDataConverter::variantToRegisters(QVariant(-1000000), "x", "DINT", BigEndian);
            ASSERT_EQUAL(regs2.size(), 2);;

            // CSV WORD -> internal "ushort" -> 1 регистр
            auto regs3 = ModbusDataConverter::variantToRegisters(QVariant(42u), "ir1", map.types[1]["ir1"], BigEndian);
            ASSERT_EQUAL(regs3.size(), 1);
        }
    }

    // 3. Невалидный CSV (неизвестный Region) — запись пропускается с warning
    {
        const QByteArray badCsv =
            "\"Name\";\"Region\";\"Address\";\"DataTypeDevice\";\"DataTypeScada\";"
            "\"Access\";\"TypeString\";\"LengthString\";\"Scale\";\"Comment\"\n"
            "\"x\";\"UNKNOWN_REGION\";\"0\";\"WORD\";\"WORD\";\"ReadWrite\";\"\";\"\";\"\";\"x\"\n"
            "\"y\";\"COILS\";\"0\";\"WORD\";\"WORD\";\"ReadWrite\";\"\";\"\";\"\";\"y\"\n" // WORD в COILS недопустим
            "\"z\";\"INPUT_REGISTERS\";\"0\";\"WORD\";\"WORD\";\"ReadOnly\";\"\";\"\";\"\";\"z\"\n";

        QTemporaryFile f;
        f.open();
        f.write(badCsv);
        f.close();

        QStringList warnings;
        auto map = ModbusCsvImporter::loadFile(1, f.fileName(), nullptr, nullptr, &warnings);

        ASSERT_EQUAL(warnings.size(), 2);            // x (region) + y (WORD в Coils)
        ASSERT_EQUAL(static_cast<int>(map.mdum[1].size()), 1); // только z
        ASSERT(map.mdum[1].count("z") == 1);
    }

    // 4. Ошибочный файл — нечитаемый
    {
        bool ok = true;
        QString error;
        auto entries = ModbusCsvImporter::parse("/nonexistent/path.csv", &ok, &error);
        ASSERT(!ok);
        ASSERT(!error.isEmpty());
        ASSERT(entries.isEmpty());
    }
}
