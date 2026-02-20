#include "ModbusRegisterSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "ModbusBase.h"

QString ModbusRegisterSerializer::serialize(const std::map<QString, QModbusDataUnit>& mdum,
                                            const std::map<QString, QString>& regDataTypes)
{
    QJsonArray array;
    for (const auto& pair : mdum) {
        const QString& name = pair.first;
        const QModbusDataUnit& unit = pair.second;
        QJsonObject obj;
        obj["RegisterName"] = name;
        obj["RegisterType"] = ModbusRegisterType_to_text(static_cast<ModbusRegisterType>(unit.registerType()));
        obj["Address"] = unit.startAddress();
        obj["DataLength"] = static_cast<int>(unit.valueCount());

        auto it = regDataTypes.find(name);
        if (it != regDataTypes.end()) {
            obj["RegisterDataType"] = it->second;
        } else {
            obj["RegisterDataType"] = "Undefined";
        }
        array.append(obj);
    }
    QJsonObject root;
    root["mdum"] = array;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

std::pair<std::map<QString, QModbusDataUnit>, std::map<QString, QString>>
ModbusRegisterSerializer::deserialize(const QString& json)
{
    std::map<QString, QModbusDataUnit> mdum;
    std::map<QString, QString> regDataTypes;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull())
        return {mdum, regDataTypes};

    QJsonArray array;
    if (doc.isObject()) {
        QJsonObject root = doc.object();
        array = root["mdum"].toArray();
    } else if (doc.isArray()) {
        array = doc.array();
    }

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
        mdum[name] = unit;
        if (!dataType.isEmpty() && dataType != "Undefined")
            regDataTypes[name] = dataType;
    }

    return {mdum, regDataTypes};
}

QJsonObject ModbusRegisterSerializer::toJsonObject(const std::map<QString, QModbusDataUnit>& mdum,
                                                   const std::map<QString, QString>& regDataTypes)
{
    QJsonArray array;
    for (const auto& pair : mdum) {
        QJsonObject obj;
        obj["RegisterName"] = pair.first;
        obj["RegisterType"] = ModbusRegisterType_to_text(static_cast<ModbusRegisterType>(pair.second.registerType()));
        obj["Address"] = pair.second.startAddress();
        obj["DataLength"] = static_cast<int>(pair.second.valueCount());

        auto it = regDataTypes.find(pair.first);
        if (it != regDataTypes.end())
            obj["RegisterDataType"] = it->second;
        array.append(obj);
    }
    QJsonObject root;
    root["mdum"] = array;
    return root;
}

void ModbusRegisterSerializer::fromJsonObject(const QJsonObject& obj,
                                              std::map<QString, QModbusDataUnit>& mdum,
                                              std::map<QString, QString>& regDataTypes)
{
    mdum.clear();
    regDataTypes.clear();
    QJsonArray array = obj["mdum"].toArray();
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
        mdum[name] = unit;
        if (!dataType.isEmpty() && dataType != "Undefined")
            regDataTypes[name] = dataType;
    }
}




void testSerializeDeserialize() {
    std::map<QString, QModbusDataUnit> mdum;
    std::map<QString, QString> regTypes;

    mdum["reg1"] = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2);
    mdum["reg2"] = QModbusDataUnit(QModbusDataUnit::InputRegisters, 10, 1);

    regTypes["reg1"] = "float";
    regTypes["reg2"] = "uint16";

    QString json = ModbusRegisterSerializer::serialize(mdum, regTypes);
    auto [mdum2, types2] = ModbusRegisterSerializer::deserialize(json);

    ASSERT_EQUAL(mdum.size(), mdum2.size());
    ASSERT_EQUAL(regTypes.size(), types2.size());

    auto it1 = mdum.find("reg1");
    auto it2 = mdum2.find("reg1");
    ASSERT(it1 != mdum.end() && it2 != mdum2.end());
    ASSERT_EQUAL(it1->second.registerType(), it2->second.registerType());
    ASSERT_EQUAL(it1->second.startAddress(), it2->second.startAddress());
    ASSERT_EQUAL(it1->second.valueCount(), it2->second.valueCount());

    ASSERT_EQUAL(types2["reg1"], "float");
    ASSERT_EQUAL(types2["reg2"], "uint16");
}

void testEmpty() {
    std::map<QString, QModbusDataUnit> mdum;
    std::map<QString, QString> regTypes;
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
