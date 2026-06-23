#ifndef BINDINGMANAGER_H
#define BINDINGMANAGER_H

#include <QList>
#include <QMap>
#include <QModbusDataUnit>
#include <QObject>
#include <QSet>

#include "RegisterBinding.h"

class ModbusClient;

class CORE_MODBUS_LIB_EXPORT BindingManager : public QObject {
    Q_OBJECT
public:
    explicit BindingManager(int serverAddress,
                            std::shared_ptr<ModbusClient> modbusClient,
                            QObject* parent = nullptr);
    ~BindingManager();

    void addBinding(std::shared_ptr<RegisterBinding> binding);
    void removeBinding(const QString& registerName);
    void removeBinding(std::shared_ptr<RegisterBinding> binding);
    void clearBindings();

    QList<std::shared_ptr<RegisterBinding>> get_bindings(const QString& registerName) const;
    QMap<QString, QList<std::shared_ptr<RegisterBinding>>> getAllBindings() const { return m_bindings; }

    std::shared_ptr<ModbusClient> getModbusClient() const { return m_modbusClient; }

    int getServerAddress() const {return m_serverAddress;};
    void setServerAddress(int serverAddress) {m_serverAddress = serverAddress;};

    void updateBindingsFromRegisters();
    void processModbusData(int serverAddress, const QModbusDataUnit& unit);
    QList<QModbusDataUnit> getPollingUnits() const;

    // Методы для управления кэшем регистров
    void updateRegisterCache(const QString& registerName, const QVector<quint16>& data);
    QVector<quint16> getRegisterCache(const QString& registerName) const;
    bool hasRegisterCache(const QString& registerName) const;

    // Метод для принудительного чтения регистра
    void requestRegisterRead(const QString& registerName);

    // Регистрация виджетов из контейнера
    void registerWidgetsFromContainer(QWidget* container);

    void loadTransactionalGroups();
    void setTransactionalGroup(const QStringList& registerNames);
signals:
    void bindingValueChanged(const QString& registerName, const QVariant& value);
    void registerValueRequested(const QString& registerName, const QVector<quint16>& data);
    void readRequested(const QString& registerName);
protected slots:
    virtual void onBindingValueChanged();
    void onRegisterValueRequested(const QString& registerName, const QVector<quint16>& data);
    void onReadRequested(const QString& registerName);
protected:
    int m_serverAddress;
    std::shared_ptr<ModbusClient> m_modbusClient;
    QMap<QString, QList<std::shared_ptr<RegisterBinding>>> m_bindings; // registerName -> bindings
    QMap<QString, QVector<quint16>> m_registerCache; // Кэш значений регистров
    QSet<RegisterBinding*> m_updatingBindings; // Привязки, которые в процессе обновления
private:
    QModbusDataUnit findPollingUnitForRegister(const QString& registerName) const;

    QModbusDataUnit buildTransactionalUnit(const QString& groupName) const;

    void scanWidgetForBindings(QWidget* widget);
    void createBindingForWidget(QWidget* widget, const QString& registerName);
    std::shared_ptr<RegisterBinding> createAppropriateBinding(QWidget* widget,
                                                              const QString& registerName,
                                                              const QString& dataType = QString());

    QMap<QString, QStringList> m_transactionalGroups; // groupName -> registerNames
    QMap<QString, QString> m_registerToGroup;         // registerName -> groupName
};

#endif // BINDINGMANAGER_H
