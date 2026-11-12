#include "ModbusSettings.h"

#include "ModbusBase.h"

QString makeModbusClientKey(const ModbusSettings& settings) {
   if (settings.MODBUS_TYPE == "TCP") {
       return QString("ModbusClient_TCP_%1_%2")
       .arg(settings.MODBUS_IP)
           .arg(settings.MODBUS_PORT);
   } else {
       return QString("ModbusClient_Serial_%1_%2_%3_%4_%5")
       .arg(settings.MODBUS_SERIAL_PORT)
           .arg(settings.MODBUS_BAUDRATE)
           .arg(settings.MODBUS_DATA_BITS)
           .arg(settings.MODBUS_STOP_BITS)
           .arg(settings.MODBUS_PARITY);
   }
}

int ModbusSettingsConverter::modbus_state_from_str(QString state_str) {
    int state = QModbusDevice::UnconnectedState;

    state_str = state_str.toLower();
    if (state_str.contains("отключено")) {
        state = QModbusDevice::UnconnectedState;
    } else if (state_str.contains("подключение")) {
        state = QModbusDevice::ConnectingState;
    } else if (state_str.contains("подключено")) {
        state = QModbusDevice::ConnectedState;
    } else if (state_str.contains("отключение")) {
        state = QModbusDevice::ClosingState;
    }

    return state;
}

QString ModbusSettingsConverter::modbus_state_to_str(int state) {
    QString state_str;
    switch ((QModbusDevice::State)state) {
    case QModbusDevice::UnconnectedState:
        state_str = "Отключено";
        break;
    case QModbusDevice::ConnectingState:
        state_str = "Подключение";
        break;
    case QModbusDevice::ConnectedState:
        state_str = "Подключено";
        break;
    case QModbusDevice::ClosingState:
        state_str = "Отключение";
        break;
    default:
        state_str = "Отключено";
    }
    return state_str;
}

ModbusSettings ModbusSettingsConverter::modbus_settings_from_str(QString data, bool exclude_state) {
    ModbusSettings modbus_settings;
    QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
    if (!document.isNull()) {
        if (document.isObject()) {
            QJsonObject modbus_settings_object = document.object();
            modbus_settings.MODBUS_ENABLE = modbus_settings_object["MODBUS_ENABLE"].toBool(modbus_settings.MODBUS_ENABLE);
            modbus_settings.MODBUS_POLL_TIMER = modbus_settings_object["MODBUS_POLL_TIMER"].toInt(modbus_settings.MODBUS_POLL_TIMER);
            modbus_settings.MODBUS_ADDRESS = modbus_settings_object["MODBUS_ADDRESS"].toInt(modbus_settings.MODBUS_ADDRESS);
            modbus_settings.MODBUS_TYPE = modbus_settings_object["MODBUS_TYPE"].toString(modbus_settings.MODBUS_TYPE);
            modbus_settings.MODBUS_IP = modbus_settings_object["MODBUS_IP"].toString(modbus_settings.MODBUS_IP);
            modbus_settings.MODBUS_PORT = modbus_settings_object["MODBUS_PORT"].toInt(modbus_settings.MODBUS_PORT);
            modbus_settings.MODBUS_SERIAL_PORT = modbus_settings_object["MODBUS_SERIAL_PORT"].toString(modbus_settings.MODBUS_SERIAL_PORT);
            modbus_settings.MODBUS_BAUDRATE = modbus_settings_object["MODBUS_BAUDRATE"].toInt(modbus_settings.MODBUS_BAUDRATE);
            modbus_settings.MODBUS_DATA_BITS = modbus_settings_object["MODBUS_DATA_BITS"].toInt(modbus_settings.MODBUS_DATA_BITS);
            modbus_settings.MODBUS_STOP_BITS = modbus_settings_object["MODBUS_STOP_BITS"].toInt(modbus_settings.MODBUS_STOP_BITS);
            modbus_settings.MODBUS_PARITY = modbus_settings_object["MODBUS_PARITY"].toString(modbus_settings.MODBUS_PARITY);
            if (!exclude_state) {
                modbus_settings.MODBUS_STATE = modbus_settings_object["MODBUS_STATE"].toString(modbus_settings.MODBUS_STATE);
            }
            modbus_settings.MODBUS_FORMAT = modbus_settings_object["MODBUS_FORMAT"].toString(modbus_settings.MODBUS_FORMAT);
        }
    }
    return modbus_settings;
}

QString ModbusSettingsConverter::modbus_settings_to_str(const ModbusSettings&  modbus_settings, bool exclude_state) {
    QJsonObject modbus_settings_object;

    modbus_settings_object.insert("MODBUS_ENABLE", QJsonValue(modbus_settings.MODBUS_ENABLE));
    modbus_settings_object.insert("MODBUS_POLL_TIMER", QJsonValue(modbus_settings.MODBUS_POLL_TIMER));
    modbus_settings_object.insert("MODBUS_ADDRESS", QJsonValue(modbus_settings.MODBUS_ADDRESS));
    modbus_settings_object.insert("MODBUS_TYPE", QJsonValue(modbus_settings.MODBUS_TYPE));
    modbus_settings_object.insert("MODBUS_IP", QJsonValue(modbus_settings.MODBUS_IP));
    modbus_settings_object.insert("MODBUS_PORT", QJsonValue(modbus_settings.MODBUS_PORT));
    modbus_settings_object.insert("MODBUS_SERIAL_PORT", QJsonValue(modbus_settings.MODBUS_SERIAL_PORT));
    modbus_settings_object.insert("MODBUS_BAUDRATE", QJsonValue(modbus_settings.MODBUS_BAUDRATE));
    modbus_settings_object.insert("MODBUS_DATA_BITS", QJsonValue(modbus_settings.MODBUS_DATA_BITS));
    modbus_settings_object.insert("MODBUS_STOP_BITS", QJsonValue(modbus_settings.MODBUS_STOP_BITS));
    modbus_settings_object.insert("MODBUS_PARITY", QJsonValue(modbus_settings.MODBUS_PARITY));
    if (!exclude_state) {
        modbus_settings_object.insert("MODBUS_STATE", QJsonValue(modbus_settings.MODBUS_STATE));
    }
    modbus_settings_object.insert("MODBUS_FORMAT", QJsonValue(modbus_settings.MODBUS_FORMAT));

    QJsonDocument document(modbus_settings_object);
    return document.toJson(QJsonDocument::Compact);
}

QString ModbusSettingsConverter::getString(const QVariantMap& map, const QString& key, const QString& defaultValue)
{
    if (map.contains(key) && map.value(key).canConvert<QString>()) {
        return map.value(key).toString();
    }
    return defaultValue;
}

int ModbusSettingsConverter::getInt(const QVariantMap& map, const QString& key, int defaultValue)
{
    if (map.contains(key) && map.value(key).canConvert<int>()) {
        return map.value(key).toInt();
    }
    return defaultValue;
}

bool ModbusSettingsConverter::getBool(const QVariantMap& map, const QString& key, bool defaultValue)
{
    if (map.contains(key) && map.value(key).canConvert<bool>()) {
        return map.value(key).toBool();
    }
    return defaultValue;
}

ModbusSettings ModbusSettingsConverter::modbus_settings_from_variant_map(const QVariantMap &map) {
    ModbusSettings modbus_settings;

    // Используем значения по умолчанию, если ключи отсутствуют
    modbus_settings.MODBUS_ENABLE = ModbusSettingsConverter::getBool(map, "MODBUS_ENABLE", true);
    modbus_settings.MODBUS_POLL_TIMER = getInt(map, "MODBUS_POLL_TIMER", 500);
    modbus_settings.MODBUS_ADDRESS = getInt(map, "MODBUS_ADDRESS", 1);
    modbus_settings.MODBUS_TYPE = getString(map, "MODBUS_TYPE", "TCP");
    modbus_settings.MODBUS_IP = getString(map, "MODBUS_IP", "192.168.1.1");
    modbus_settings.MODBUS_PORT = getInt(map, "MODBUS_PORT", 32767);
    modbus_settings.MODBUS_SERIAL_PORT = getString(map, "MODBUS_SERIAL_PORT", "COM1");
    modbus_settings.MODBUS_BAUDRATE = getInt(map, "MODBUS_BAUDRATE", 115200);
    modbus_settings.MODBUS_DATA_BITS = getInt(map, "MODBUS_DATA_BITS", 8);
    modbus_settings.MODBUS_STOP_BITS = getInt(map, "MODBUS_STOP_BITS", 1);
    modbus_settings.MODBUS_PARITY = getString(map, "MODBUS_PARITY", "NoParity");
    modbus_settings.MODBUS_STATE = getString(map, "MODBUS_STATE", "");
    modbus_settings.MODBUS_FORMAT = getString(map, "MODBUS_FORMAT", "ABCD");

    return modbus_settings;
}

QVariantMap ModbusSettingsConverter::modbus_settings_to_variant_map(const ModbusSettings&  modbus_settings) {
    QVariantMap map;

    map["MODBUS_ENABLE"] = modbus_settings.MODBUS_ENABLE;
    map["MODBUS_POLL_TIMER"] = modbus_settings.MODBUS_POLL_TIMER;
    map["MODBUS_ADDRESS"] = modbus_settings.MODBUS_ADDRESS;
    map["MODBUS_TYPE"] = modbus_settings.MODBUS_TYPE;
    map["MODBUS_IP"] = modbus_settings.MODBUS_IP;
    map["MODBUS_PORT"] = modbus_settings.MODBUS_PORT;
    map["MODBUS_SERIAL_PORT"] = modbus_settings.MODBUS_SERIAL_PORT;
    map["MODBUS_BAUDRATE"] = modbus_settings.MODBUS_BAUDRATE;
    map["MODBUS_DATA_BITS"] = modbus_settings.MODBUS_DATA_BITS;
    map["MODBUS_STOP_BITS"] = modbus_settings.MODBUS_STOP_BITS;
    map["MODBUS_PARITY"] = modbus_settings.MODBUS_PARITY;
    map["MODBUS_STATE"] = modbus_settings.MODBUS_STATE;
    map["MODBUS_FORMAT"] = modbus_settings.MODBUS_FORMAT;

    return map;
}
