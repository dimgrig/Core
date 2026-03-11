#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <QString>
#include <typeindex>
#include <map>

#include <QObject>
#include <QStringList>

#include "core-io-lib-global.h"

class ModbusClient;
class HTTPClient;
class TCPClient;
class UDPClient;
class SerialClient;
class MQTTClient;

struct ModbusSettings;

#define rm ResourceManager::instance()

class CORE_IO_LIB_EXPORT ResourceManager {
public:
    static ResourceManager& instance() {
        static ResourceManager instance;
        return instance;
    }

    // Основной шаблонный метод для получения любого клиента
    template<typename ClientType, typename... Args>
    std::shared_ptr<ClientType> get_client(const QString& resource_id = "default",
                                           Args&&... args) {
        std::lock_guard<std::mutex> lock(get_mutex<ClientType>());

        auto& clients_map = get_clients_map<ClientType>();
        auto it = clients_map.find(resource_id);

        if (it != clients_map.end()) {
            return std::static_pointer_cast<ClientType>(it->second);
        }

        // Создаем новый клиент
        auto client = std::make_shared<ClientType>(std::forward<Args>(args)...);
        clients_map[resource_id] = client;
        return client;
    }

    // Проверка существования клиента
    template<typename ClientType>
    bool has_client(const QString& resource_id = "default") const {
        std::lock_guard<std::mutex> lock(get_mutex<ClientType>());

        const auto& clients_map = get_clients_map<ClientType>();
        return clients_map.find(resource_id) != clients_map.end();
    }

    // Удаление клиента
    template<typename ClientType>
    void remove_client(const QString& resource_id = "default") {
        std::lock_guard<std::mutex> lock(get_mutex<ClientType>());

        auto& clients_map = get_clients_map<ClientType>();
        clients_map.erase(resource_id);
    }

    // Получить или создать клиент по умолчанию с настройками
    template<typename ClientType>
    std::shared_ptr<ClientType> get_or_create_default() {
        return get_client<ClientType>("default");
    }

    // Очистить все клиенты определенного типа
    template<typename ClientType>
    void clear_clients() {
        std::lock_guard<std::mutex> lock(get_mutex<ClientType>());

        auto& clients_map = get_clients_map<ClientType>();
        clients_map.clear();
    }

    // Очистить все ресурсы
    void clear_all() {
        std::lock_guard<std::mutex> lock(main_mutex_);
        clients_maps_.clear();
        mutexes_.clear();
    }

    // Получить список всех ресурсов определенного типа
    template<typename ClientType>
    QStringList get_client_ids() const {
        std::lock_guard<std::mutex> lock(get_mutex<ClientType>());

        QStringList ids;
        const auto& clients_map = get_clients_map<ClientType>();

        for (const auto& pair : clients_map) {
            ids.append(pair.first);
        }

        return ids;
    }

private:
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ~ResourceManager() {};

    // Основное хранилище: type_index -> map<QString, shared_ptr<void>>
    std::map<std::type_index,
             std::unordered_map<QString, std::shared_ptr<void>>> clients_maps_;

    // Мьютексы для каждого типа
    mutable std::map<std::type_index, std::mutex> mutexes_;

    // Главный мьютекс для изменения контейнеров
    mutable std::mutex main_mutex_;

    // Получение карты клиентов для типа (создает при необходимости)
    template<typename ClientType>
    std::unordered_map<QString, std::shared_ptr<void>>& get_clients_map() {
        std::lock_guard<std::mutex> lock(main_mutex_);

        auto type_idx = std::type_index(typeid(ClientType));
        return clients_maps_[type_idx];
    }

    template<typename ClientType>
    const std::unordered_map<QString, std::shared_ptr<void>>& get_clients_map() const {
        std::lock_guard<std::mutex> lock(main_mutex_);

        auto type_idx = std::type_index(typeid(ClientType));
        auto it = clients_maps_.find(type_idx);
        if (it != clients_maps_.end()) {
            return it->second;
        }

        static const std::unordered_map<QString, std::shared_ptr<void>> empty_map;
        return empty_map;
    }

    // Получение мьютекса для типа (создает при необходимости)
    template<typename ClientType>
    std::mutex& get_mutex() const {
        std::lock_guard<std::mutex> lock(main_mutex_);

        auto type_idx = std::type_index(typeid(ClientType));
        return mutexes_[type_idx];
    }
};

#endif // RESOURCEMANAGER_H
