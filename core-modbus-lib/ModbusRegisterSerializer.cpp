#include "ModbusRegisterSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "ModbusBase.h"

QString ModbusRegisterSerializer::serialize(const std::map <int, std::map<QString, QModbusDataUnit>>& mdum,
                                            const std::map <int, std::map<QString, QString>>& regDataTypes)
{
    QJsonArray devicesArray;
    for (const auto& [serverAddress, _mdum] : mdum) {
        QJsonObject deviceObj;
        deviceObj["ServerAddress"] = serverAddress;

        QJsonArray array;
        for (const auto& pair : _mdum) {
            const QString& name = pair.first;
            const QModbusDataUnit& unit = pair.second;
            QJsonObject obj;
            obj["RegisterName"] = name;
            obj["RegisterType"] = ModbusRegisterType_to_text(static_cast<ModbusRegisterType>(unit.registerType()));
            obj["Address"] = unit.startAddress();
            obj["DataLength"] = static_cast<int>(unit.valueCount());

            if (regDataTypes.count(serverAddress)) {
                auto it = regDataTypes.at(serverAddress).find(name);
                if (it != regDataTypes.at(serverAddress).end()) {
                    obj["RegisterDataType"] = it->second;
                } else {
                    obj["RegisterDataType"] = "Undefined";
                }
            } else {
                obj["RegisterDataType"] = "Undefined";
            }
            array.append(obj);
        }

        deviceObj["Registers"] = array;
        devicesArray.append(deviceObj);
    }
    QJsonObject root;
    root["mdum"] = devicesArray;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

std::pair<std::map <int, std::map<QString, QModbusDataUnit>>, std::map <int, std::map<QString, QString>>>
ModbusRegisterSerializer::deserialize(const QString& json)
{
    std::map <int, std::map<QString, QModbusDataUnit>> mdum;
    std::map <int, std::map<QString, QString>> regDataTypes;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull())
        return {mdum, regDataTypes};

    QJsonArray devicesArray;

    if (doc.isObject()) {
        QJsonObject root = doc.object();
        devicesArray = root["mdum"].toArray();

        // Проверяем, является ли массив списком регистров (старый формат)
        if (!devicesArray.isEmpty()) {
            QJsonValue first = devicesArray.first();
            if (first.isObject() && !first.toObject().contains("ServerAddress") && !first.toObject().contains("Registers")) {
                // Это старый формат: массив регистров -> оборачиваем в адрес 1
                QJsonObject wrapper;
                wrapper["ServerAddress"] = 1;
                wrapper["Registers"] = devicesArray;
                devicesArray = QJsonArray();
                devicesArray.append(wrapper);
            }
        }
    } else if (doc.isArray()) {
        QJsonArray arr = doc.array();
        if (arr.isEmpty())
            return {mdum, regDataTypes};

        QJsonValue first = arr.first();
        if (first.isObject() && (first.toObject().contains("ServerAddress") || first.toObject().contains("Registers"))) {
            devicesArray = doc.array();
        } else {
            // старый формат: массив регистров без serverAddress
            QJsonArray oldArray = doc.array();
            QJsonObject wrapper;
            wrapper["ServerAddress"] = 1;
            wrapper["Registers"] = oldArray;
            devicesArray.append(wrapper);
        }
    } else {
        return {mdum, regDataTypes};
    }

    if (devicesArray.isEmpty())
        return {mdum, regDataTypes};

    for (const QJsonValue& deviceVal : devicesArray) {
        QJsonObject deviceObj = deviceVal.toObject();
        int serverAddress = deviceObj["ServerAddress"].toInt(1); // по умолчанию 1 (обратная совместимость)
        QJsonArray array = deviceObj["Registers"].toArray();

        for (const QJsonValue& val : array) {
            QJsonObject obj = val.toObject();
            QString name = obj["RegisterName"].toString();
            QString regTypeStr = obj["RegisterType"].toString();
            int address = obj["Address"].toInt();
            int length = obj["DataLength"].toInt();
            QString dataType = obj["RegisterDataType"].toString();

            ModbusRegisterType regType = ModbusRegisterType_from_text(regTypeStr.toStdString().c_str());
            QModbusDataUnit::RegisterType qType = static_cast<QModbusDataUnit::RegisterType>(regType);
            QModbusDataUnit unit(qType, address, static_cast<quint16>(length));
            mdum[serverAddress][name] = unit;
            if (!dataType.isEmpty() && dataType != "Undefined")
                regDataTypes[serverAddress][name] = dataType;
        }
    }

    return {mdum, regDataTypes};
}

QJsonObject ModbusRegisterSerializer::toJsonObject(const std::map <int, std::map<QString, QModbusDataUnit>>& mdum,
                                                   const std::map <int, std::map<QString, QString>>& regDataTypes)
{
    QJsonArray devicesArray;
    for (const auto& [serverAddress, _mdum] : mdum) {
        QJsonObject deviceObj;
        deviceObj["ServerAddress"] = serverAddress;

        QJsonArray array;
        for (const auto& pair : _mdum) {
            QJsonObject obj;
            obj["RegisterName"] = pair.first;
            obj["RegisterType"] = ModbusRegisterType_to_text(static_cast<ModbusRegisterType>(pair.second.registerType()));
            obj["Address"] = pair.second.startAddress();
            obj["DataLength"] = static_cast<int>(pair.second.valueCount());

            if (regDataTypes.count(serverAddress)) {
                auto it = regDataTypes.at(serverAddress).find(pair.first);
                if (it != regDataTypes.at(serverAddress).end())
                    obj["RegisterDataType"] = it->second;
                array.append(obj);
            }
        }
        deviceObj["Registers"] = array;
        devicesArray.append(deviceObj);
    }
    QJsonObject root;
    root["mdum"] = devicesArray;
    return root;
}

void ModbusRegisterSerializer::fromJsonObject(const QJsonObject& obj,
                                              std::map <int, std::map<QString, QModbusDataUnit>>& mdum,
                                              std::map <int, std::map<QString, QString>>& regDataTypes)
{
    mdum.clear();
    regDataTypes.clear();
    QJsonArray devicesArray = obj["mdum"].toArray();
    for (const QJsonValue& deviceVal : devicesArray) {
        QJsonObject deviceObj = deviceVal.toObject();
        int serverAddress = deviceObj["ServerAddress"].toInt(1);
        QJsonArray array = deviceObj["Registers"].toArray();
        for (const QJsonValue& val : array) {
            QJsonObject o = val.toObject();
            QString name = o["RegisterName"].toString();
            QString regTypeStr = o["RegisterType"].toString();
            int address = o["Address"].toInt();
            int length = o["DataLength"].toInt();
            QString dataType = o["RegisterDataType"].toString();

            ModbusRegisterType regType = ModbusRegisterType_from_text(regTypeStr.toStdString().c_str());
            QModbusDataUnit::RegisterType qType = static_cast<QModbusDataUnit::RegisterType>(regType);
            QModbusDataUnit unit(qType, address, static_cast<quint16>(length));
            mdum[serverAddress][name] = unit;
            if (!dataType.isEmpty() && dataType != "Undefined")
                regDataTypes[serverAddress][name] = dataType;
        }
    }
}

void testSerializeDeserialize() {
    int serverAddress = 1;

    std::map <int, std::map<QString, QModbusDataUnit>> mdum;
    std::map <int, std::map<QString, QString>> regTypes;

    mdum[serverAddress]["reg1"] = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2);
    mdum[serverAddress]["reg2"] = QModbusDataUnit(QModbusDataUnit::InputRegisters, 10, 1);

    regTypes[serverAddress]["reg1"] = "float";
    regTypes[serverAddress]["reg2"] = "uint16";

    QString json = ModbusRegisterSerializer::serialize(mdum, regTypes);
    auto [mdum2, types2] = ModbusRegisterSerializer::deserialize(json);

    ASSERT_EQUAL(mdum.size(), mdum2.size());
    ASSERT_EQUAL(regTypes.size(), types2.size());

    auto it1 = mdum.at(serverAddress).find("reg1");
    auto it2 = mdum2.at(serverAddress).find("reg1");
    ASSERT(it1 != mdum.at(serverAddress).end() && it2 != mdum2.at(serverAddress).end());
    ASSERT_EQUAL(it1->second.registerType(), it2->second.registerType());
    ASSERT_EQUAL(it1->second.startAddress(), it2->second.startAddress());
    ASSERT_EQUAL(it1->second.valueCount(), it2->second.valueCount());

    ASSERT_EQUAL(types2.at(serverAddress)["reg1"], "float");
    ASSERT_EQUAL(types2.at(serverAddress)["reg2"], "uint16");
}

void testEmpty() {
    std::map <int, std::map<QString, QModbusDataUnit>> mdum;
    std::map <int, std::map<QString, QString>> regTypes;
    QString json = ModbusRegisterSerializer::serialize(mdum, regTypes);
    auto [mdum2, types2] = ModbusRegisterSerializer::deserialize(json);
    ASSERT(mdum2.empty());
    ASSERT(types2.empty());
}

void testMalformedJson() {
    QString badJson = "{ not json }";
    auto [mdum, types] = ModbusRegisterSerializer::deserialize(badJson);
    ASSERT(mdum.empty());
    ASSERT(types.empty());
}

void ModbusRegisterSerializerTest() {
    testSerializeDeserialize();
    testEmpty();
    testMalformedJson();
}
