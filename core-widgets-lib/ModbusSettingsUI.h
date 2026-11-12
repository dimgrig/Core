#ifndef MODBUSCLIENTSETTINGSUI_H
#define MODBUSCLIENTSETTINGSUI_H

#include <QWidget>

#include <core-modbus-lib/ModbusSettings.h>

#include "core-widgets-lib-global.h"

namespace Ui { class fmodbus_settings; }

class CORE_WIDGETS_LIB_EXPORT ModbusSettingsUI : public QWidget {
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

#endif // MODBUSCLIENTSETTINGSUI_H
