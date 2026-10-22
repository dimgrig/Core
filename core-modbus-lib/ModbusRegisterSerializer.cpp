#include "ModbusRegisterSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "ModbusBase.h"

static std::map<int, std::map<QString, QString>> buildRegisterToGroup(
    const std::map<int, std::map<QString, QStringList>>& transactionalGroup)
{
    std::map<int, std::map<QString, QString>> reg2group;
    for (const auto& [addr, group] : transactionalGroup) {
        for (const auto& [groupName, regNames] : group) {
            for (const QString& reg : regNames) {
                reg2group[addr][reg] = groupName;
            }
        }
    }
    return reg2group;
}

QString ModbusRegisterSerializer::serialize(int serverAddress, const ModbusRegisterMap& map) {
    QJsonObject root = toJsonObject(serverAddress, map);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QJsonObject ModbusRegisterSerializer::toJsonObject(int serverAddress, const ModbusRegisterMap& map) {
    Q_UNUSED(serverAddress);
    auto regToGroup = buildRegisterToGroup(map.groups);
    QJsonArray array;
    for (const auto& [addr, _mdum] : map.mdum) {
        for (const auto& [name, unit] : _mdum) {
            QJsonObject obj;
            obj["RegisterName"] = name;
            obj["RegisterType"] = ModbusRegisterType_to_text(static_cast<ModbusRegisterType>(unit.registerType()));
            obj["Address"]      = unit.startAddress();
            obj["DataLength"]   = static_cast<int>(unit.valueCount());

            if (map.types.count(addr) && map.types.at(addr).count(name))
                obj["RegisterDataType"] = map.types.at(addr).at(name);
            else
                obj["RegisterDataType"] = "Undefined";

            if (map.access.count(addr) && map.access.at(addr).count(name))
                obj["Access"] = ModbusRegisterAccess_to_text(map.access.at(addr).at(name));
            else
                obj["Access"] = "ReadWrite";

            if (map.scales.count(addr) && map.scales.at(addr).count(name))
                obj["Scale"] = map.scales.at(addr).at(name);

            if (regToGroup.count(addr) && regToGroup[addr].count(name))
                obj["TransactionGroup"] = regToGroup[addr][name];

            array.append(obj);
        }
    }
    QJsonObject root;
    root["mdum"] = array;
    return root;
}

ModbusRegisterMap ModbusRegisterSerializer::deserialize(int serverAddress, const QString& json) {
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull()) return {};
    if (doc.isObject()) return fromJsonObject(serverAddress, doc.object());
    if (doc.isArray()) {
        QJsonObject root;
        root["mdum"] = doc.array();
        return fromJsonObject(serverAddress, root);
    }
    return {};
}

ModbusRegisterMap ModbusRegisterSerializer::fromJsonObject(int serverAddress, const QJsonObject& obj) {
    ModbusRegisterMap result;
    const QJsonArray array = obj["mdum"].toArray();

    for (const QJsonValue& val : array) {
        const QJsonObject o = val.toObject();
        const QString name = o["RegisterName"].toString();
        const QString regTypeStr = o["RegisterType"].toString();
        const int address = o["Address"].toInt();
        const int length  = o["DataLength"].toInt();
        const QString dataType = o["RegisterDataType"].toString();

        ModbusRegisterType regType = ModbusRegisterType_from_text(regTypeStr.toStdString().c_str());
        QModbusDataUnit::RegisterType qType = static_cast<QModbusDataUnit::RegisterType>(regType);
        QModbusDataUnit unit(qType, address, static_cast<quint16>(length));
        result.mdum[serverAddress][name] = unit;

        if (!dataType.isEmpty() && dataType != "Undefined")
            result.types[serverAddress][name] = dataType;

        const QString accessStr = o["Access"].toString();
        result.access[serverAddress][name] = ModbusRegisterAccess_from_text(accessStr.toStdString().c_str());

        if (o.contains("Scale") && !o["Scale"].isNull())
            result.scales[serverAddress][name] = o["Scale"].toDouble();

        if (o.contains("TransactionGroup")) {
            const QString groupName = o["TransactionGroup"].toString();
            if (!groupName.isEmpty())
                result.groups[serverAddress][groupName].append(name);
        }
    }

    // Удаляем группы с 1 регистром — они не имеют смысла для атомарной записи
    for (auto& [addr, grp] : result.groups) {
        Q_UNUSED(addr);
        for (auto it = grp.begin(); it != grp.end(); ) {
            if (it->second.size() < 2) it = grp.erase(it);
            else ++it;
        }
    }

    return result;
}

// Тесты — оставляем прежние (адаптируем к новой сигнатуре)
void testSerializeDeserialize() {
    int serverAddress = 1;
    ModbusRegisterMap map;
    map.mdum[serverAddress]["reg1"] = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2);
    map.mdum[serverAddress]["reg2"] = QModbusDataUnit(QModbusDataUnit::InputRegisters, 10, 1);
    map.types[serverAddress]["reg1"] = "float";
    map.types[serverAddress]["reg2"] = "uint16";
    map.access[serverAddress]["reg1"] = ModbusRegisterAccess_ReadWrite;
    map.access[serverAddress]["reg2"] = ModbusRegisterAccess_ReadOnly;
    map.scales[serverAddress]["reg1"] = 0.1;
    map.groups[serverAddress]["groupA"] = QStringList{"reg1", "reg2"};

    QString json = ModbusRegisterSerializer::serialize(serverAddress, map);
    ModbusRegisterMap m2 = ModbusRegisterSerializer::deserialize(serverAddress, json);

    ASSERT_EQUAL(m2.mdum.size(), map.mdum.size());
    ASSERT_EQUAL(m2.types.size(), map.types.size());
    ASSERT_EQUAL(m2.access.size(), map.access.size());
    ASSERT_EQUAL(m2.scales.size(), map.scales.size());
    ASSERT_EQUAL(m2.types.at(serverAddress)["reg1"], "float");
    ASSERT_EQUAL(m2.access.at(serverAddress)["reg2"], ModbusRegisterAccess_ReadOnly);
    ASSERT(m2.scales.at(serverAddress)["reg1"] > 0.099 && m2.scales.at(serverAddress)["reg1"] < 0.101);
}

void testEmpty() {
    ModbusRegisterMap empty;
    QString json = ModbusRegisterSerializer::serialize(1, empty);
    auto m2 = ModbusRegisterSerializer::deserialize(1, json);
    ASSERT(m2.mdum.empty());
}

void testMalformedJson() {
    auto m = ModbusRegisterSerializer::deserialize(1, "{ not json }");
    ASSERT(m.mdum.empty());
}

void ModbusRegisterSerializerTest() {
    testSerializeDeserialize();
    testEmpty();
    testMalformedJson();
}
