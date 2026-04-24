#ifndef MODBUSREGISTERSERIALIZER_H
#define MODBUSREGISTERSERIALIZER_H

#include <QString>
#include <QMap>
#include <QModbusDataUnit>
#include <QJsonObject>
#include <QJsonArray>

#include "core-modbus-lib-global.h"

CORE_MODBUS_LIB_EXPORT void ModbusRegisterSerializerTest();

class CORE_MODBUS_LIB_EXPORT ModbusRegisterSerializer {
public:
    // Сериализует карту регистров и типов данных в JSON строку
    static QString serialize(int serverAddress,
                             const std::map <int, std::map<QString, QModbusDataUnit>>& mdum,
                             const std::map <int, std::map<QString, QString>>& regDataTypes);

    // Десериализует JSON строку в пару (mdum, regDataTypes)
    static std::pair<std::map <int, std::map<QString, QModbusDataUnit>>,
                     std::map <int, std::map<QString, QString>>> deserialize(int serverAddress, const QString& json);

    // Вспомогательные методы для работы с QJsonObject
    static QJsonObject toJsonObject(int serverAddress,
                                    const std::map <int, std::map<QString, QModbusDataUnit>>& mdum,
                                    const std::map <int, std::map<QString, QString>>& regDataTypes);
    static void fromJsonObject(int serverAddress, const QJsonObject& obj,
                               std::map <int, std::map<QString, QModbusDataUnit>>& mdum,
                               std::map <int, std::map<QString, QString>>& regDataTypes);
};

#endif // MODBUSREGISTERSERIALIZER_H
