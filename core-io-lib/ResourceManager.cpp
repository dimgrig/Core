#include "ResourceManager.h"

#include "ModbusSettings.h"
#include "ModbusClient.h"

ResourceManager::ResourceManager() {

}

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance;
    return instance;
}

std::shared_ptr<ModbusClient> ResourceManager::getModbusClient(const QString &config, const ModbusSettings& settings, QWidget* parent) {
    QString key = makeModbusClientKey(settings);
    return acquire_client<ModbusClient>(key, parent, config, settings);
}

// // Реализация упрощенного метода для ModbusClient
// std::shared_ptr<ModbusClient> ResourceManager::get_modbus_client(
//     const QString& resource_id,
//     QWidget* parent,
//     const QString& config,
//     ModbusSettings modbus_settings) {

//     return get_client<ModbusClient>(resource_id, parent, config, modbus_settings);
// }
