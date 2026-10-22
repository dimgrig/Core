#ifndef MODBUSCLIENTSETTINGS_H
#define MODBUSCLIENTSETTINGS_H

#include <QWidget>

#include "core-modbus-lib-global.h"

namespace Ui { class fmodbus_settings; }

struct ModbusSettings {
    bool MODBUS_ENABLE = true;
    int MODBUS_POLL_TIMER = 1000;
    int MODBUS_ADDRESS = 1;
    QString MODBUS_TYPE = "TCP";
    QString MODBUS_IP = "192.168.1.1";
    int MODBUS_PORT = 502;
    QString MODBUS_SERIAL_PORT = "COM1";
    int MODBUS_BAUDRATE = 115200;
    int MODBUS_DATA_BITS = 8;
    int MODBUS_STOP_BITS = 1;
    QString MODBUS_PARITY = "NoParity";
    QString MODBUS_STATE = "Подключение";
    QString MODBUS_FORMAT = "ABCD";
};

CORE_MODBUS_LIB_EXPORT QString makeModbusClientKey(const ModbusSettings& settings);

class CORE_MODBUS_LIB_EXPORT ModbusSettingsConverter
{
public:
    static int modbus_state_from_str(QString state_str);
    static QString modbus_state_to_str(int state);

    static ModbusSettings modbus_settings_from_str(QString data, bool exclude_state = false);
    static QString modbus_settings_to_str(const ModbusSettings&  modbus_settings, bool exclude_state = false);

    static QVariantMap modbus_settings_to_variant_map(const ModbusSettings& modbus_settings);
    static ModbusSettings modbus_settings_from_variant_map(const QVariantMap& map);

    static bool isValidVariantMap(const QVariantMap& map);
    static void applyDefaults(ModbusSettings& modbus_settings);

    // Утилиты для работы с отдельными полями
    static QString getString(const QVariantMap& map, const QString& key, const QString& defaultValue = "");
    static int getInt(const QVariantMap& map, const QString& key, int defaultValue = 0);
    static bool getBool(const QVariantMap& map, const QString& key, bool defaultValue = false);
};

class CORE_MODBUS_LIB_EXPORT ModbusSettingsUI : public QWidget {
    Q_OBJECT
public:
    ModbusSettingsUI(QWidget *parent = nullptr);
    ~ModbusSettingsUI();

    void set_reconfig(bool reconfig = false);
    bool get_reconfig() const {return _reconfig;};
signals:
    void settings_changed();
    void config_changed(QString path);
    void poll_timer_changed(int value);

public slots:
    void type_changed(const QString &type);

    ModbusSettings getSettingsFromUi();
    void setSettingsToUi(const ModbusSettings& settings);

    int get_poll_timer() const;

    void onModbusStateChanged(int state);

    void read_config(ModbusSettings settings);
    ModbusSettings write_config();

    void load_config();
    void set_config(QString config);

    void setSettingsEnabled(bool enabled);
private slots:

private:
    void initializeComboboxes();

    Ui::fmodbus_settings *ui;

    bool _reconfig = false;
};

#endif // MODBUSCLIENTSETTINGS_H
