#include "ModbusRegisterSerializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "ModbusBase.h"

static std::map<int, std::map<QString, QString>> buildRegisterToGroup(const std::map<int, std::map<QString, QStringList>>& transactionalGroup) {
    std::map<int, std::map<QString, QString>> reg2group;
    for (const auto& [serverAddress, _transactionalGroup] : transactionalGroup) {
        for (const auto& [groupName, regNames] : _transactionalGroup) {
            for (const QString& reg : regNames) {
                reg2group[serverAddress][reg] = groupName;
            }
        }
    }
    return reg2group;
}

QString ModbusRegisterSerializer::serialize(int serverAddress,
                                            const std::map<int, std::map<QString, QModbusDataUnit>>& mdum,
                                            const std::map<int, std::map<QString, QString>>& regDataTypes,
                                            const std::map<int, std::map<QString, QStringList>>& transactionalGroup)
{
    Q_UNUSED(serverAddress);
    auto regToGroup = buildRegisterToGroup(transactionalGroup);
    QJsonArray array;
    for (const auto& [serverAddress, _mdum] : mdum) {
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

            if (regToGroup.count(serverAddress)) {
                if (regToGroup[serverAddress].count(name)) {
                    obj["TransactionGroup"] = regToGroup[serverAddress][name];
                }
            }

            array.append(obj);
        }
    }
    QJsonObject root;
    root["mdum"] = array;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

std::tuple<
    std::map<int, std::map<QString, QModbusDataUnit>>,
    std::map<int, std::map<QString, QString>>,
    std::map<int, std::map<QString, QStringList>>
    >
ModbusRegisterSerializer::deserialize(int serverAddress, const QString& json)
{
    std::map<int, std::map<QString, QModbusDataUnit>> mdum;
    std::map<int, std::map<QString, QString>> regDataTypes;
    std::map<int, std::map<QString, QStringList>> transactionalGroup;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull())
        return {mdum, regDataTypes, transactionalGroup};

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
        mdum[serverAddress][name] = unit;
        if (!dataType.isEmpty() && dataType != "Undefined")
            regDataTypes[serverAddress][name] = dataType;

        if (obj.contains("TransactionGroup")) {
            QString groupName = obj["TransactionGroup"].toString();
            if (!groupName.isEmpty()) {
                transactionalGroup[serverAddress][groupName].append(name);
            }
        }
    }

    // Удаляем группы с одним регистром (они не имеют смысла для атомарной записи)
    for (auto& [serverAddress, _transactionalGroup] : transactionalGroup) {
        for (auto it = _transactionalGroup.begin(); it != _transactionalGroup.end(); ) {
            if (it->second.size() < 2)
                it = _transactionalGroup.erase(it);
            else
                ++it;
        }
    }

    return {mdum, regDataTypes, transactionalGroup};
}

QJsonObject ModbusRegisterSerializer::toJsonObject(int serverAddress,
                                                   const std::map<int, std::map<QString, QModbusDataUnit>>& mdum,
                                                   const std::map<int, std::map<QString, QString>>& regDataTypes,
                                                   const std::map<int, std::map<QString, QStringList>>& transactionalGroup)
{
    Q_UNUSED(serverAddress)
    auto regToGroup = buildRegisterToGroup(transactionalGroup);
    QJsonArray array;
    for (const auto& [serverAddress, _mdum] : mdum) {
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

            if (regToGroup.count(serverAddress)) {
                if (regToGroup[serverAddress].count(pair.first)) {
                    obj["TransactionGroup"] = regToGroup[serverAddress][pair.first];
                }
            }
        }
    }
    QJsonObject root;
    root["mdum"] = array;
    return root;
}

void ModbusRegisterSerializer::fromJsonObject(int serverAddress, const QJsonObject& obj,
                                              std::map<int, std::map<QString, QModbusDataUnit>>& mdum,
                                              std::map<int, std::map<QString, QString>>& regDataTypes,
                                              std::map<int, std::map<QString, QStringList>>& transactionalGroup)
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
        mdum[serverAddress][name] = unit;
        if (!dataType.isEmpty() && dataType != "Undefined")
            regDataTypes[serverAddress][name] = dataType;

        if (o.contains("TransactionGroup")) {
            QString groupName = o["TransactionGroup"].toString();
            if (!groupName.isEmpty()) {
                transactionalGroup[serverAddress][groupName].append(name);
            }
        }
    }

    // Удаляем группы с одним регистром (они не имеют смысла для атомарной записи)
    for (auto& [serverAddress, _transactionalGroup] : transactionalGroup) {
        for (auto it = _transactionalGroup.begin(); it != _transactionalGroup.end(); ) {
            if (it->second.size() < 2)
                it = _transactionalGroup.erase(it);
            else
                ++it;
        }
    }
}

void testSerializeDeserialize() {
    int serverAddress = 1;

    std::map<int, std::map<QString, QModbusDataUnit>> mdum;
    std::map<int, std::map<QString, QString>> regTypes;
    std::map<int, std::map<QString, QStringList>> groups;

    mdum[serverAddress]["reg1"] = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2);
    mdum[serverAddress]["reg2"] = QModbusDataUnit(QModbusDataUnit::InputRegisters, 10, 1);
    groups[serverAddress]["groupA"] = QStringList{"reg1", "reg2"};

    regTypes[serverAddress]["reg1"] = "float";
    regTypes[serverAddress]["reg2"] = "uint16";

    QString json = ModbusRegisterSerializer::serialize(serverAddress, mdum, regTypes, groups);
    auto [mdum2, types2, groups2] = ModbusRegisterSerializer::deserialize(serverAddress, json);

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

    // Проверка групп
    ASSERT_EQUAL(groups2.size(), 1u);
    ASSERT(groups2.at(serverAddress).count("groupA"));
    ASSERT_EQUAL(groups2.at(serverAddress)["groupA"].size(), 2);
    ASSERT(groups2.at(serverAddress)["groupA"].contains("reg1"));
    ASSERT(groups2.at(serverAddress)["groupA"].contains("reg2"));
}

void testEmpty() {
    int serverAddress = 1;
    std::map<int, std::map<QString, QModbusDataUnit>> mdum;
    std::map<int, std::map<QString, QString>> regTypes;
    std::map<int, std::map<QString, QStringList>> groups;
    QString json = ModbusRegisterSerializer::serialize(serverAddress, mdum, regTypes, groups);
    auto [mdum2, types2, groups2] = ModbusRegisterSerializer::deserialize(serverAddress, json);
    ASSERT(mdum2.empty());
    ASSERT(types2.empty());
    ASSERT(groups2.empty());
}

void testMalformedJson() {
    int serverAddress = 1;
    QString badJson = "{ not json }";
    auto [mdum, types, groups] = ModbusRegisterSerializer::deserialize(serverAddress, badJson);
    ASSERT(mdum.empty());
    ASSERT(types.empty());
    ASSERT(groups.empty());
}

void ModbusRegisterSerializerTest() {
    testSerializeDeserialize();
    testEmpty();
    testMalformedJson();
}
