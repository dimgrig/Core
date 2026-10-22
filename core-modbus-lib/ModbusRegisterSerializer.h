#ifndef MODBUSREGISTERSERIALIZER_H
#define MODBUSREGISTERSERIALIZER_H

#include <QString>
#include <QMap>
#include <QModbusDataUnit>
#include <QJsonObject>
#include <QJsonArray>

#include "core-modbus-lib-global.h"

#include "ModbusBase.h"

CORE_MODBUS_LIB_EXPORT void ModbusRegisterSerializerTest();

struct CORE_MODBUS_LIB_EXPORT ModbusRegisterMap {
    std::map<int, std::map<QString, QModbusDataUnit>>            mdum;
    std::map<int, std::map<QString, QString>>                    types;
    std::map<int, std::map<QString, ModbusRegisterAccess>>       access;
    std::map<int, std::map<QString, double>>                     scales;
    std::map<int, std::map<QString, QStringList>>                groups;
};

class CORE_MODBUS_LIB_EXPORT ModbusRegisterSerializer {
public:
    // Сериализует карту регистров и типов данных в JSON строку
    static QString serialize(int serverAddress, const ModbusRegisterMap& map);

    // Десериализует JSON строку в пару (mdum, regDataTypes)
    static ModbusRegisterMap deserialize(int serverAddress, const QString& json);

    // Вспомогательные методы для работы с QJsonObject
    static QJsonObject toJsonObject(int serverAddress, const ModbusRegisterMap& map);
    static ModbusRegisterMap fromJsonObject(int serverAddress, const QJsonObject& obj);
};

#endif // MODBUSREGISTERSERIALIZER_H
