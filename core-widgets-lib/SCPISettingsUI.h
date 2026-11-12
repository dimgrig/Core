#ifndef SCPISETTINGSUI_H
#define SCPISETTINGSUI_H

#include <QWidget>
#include <QMessageBox>

#include <core-io-lib/SCPISettings.h>
#include <core-io-lib/ScpiDevice.h>

#include "core-widgets-lib-global.h"

namespace Ui { class fscpi_settings; }

class CORE_WIDGETS_LIB_EXPORT SCPISettingsUI : public QWidget {
    Q_OBJECT
public:
    SCPISettingsUI(QWidget *parent = nullptr);
    ~SCPISettingsUI();

    // void set_reconfig(bool reconfig = false);
    // bool get_reconfig() const {return _reconfig;};
signals:
    void settings_changed();

public slots:
    void type_changed(const QString &type);

    SCPISettings getSettingsFromUi();
    void setSettingsToUi(const SCPISettings& settings);

    void read_config(SCPISettings settings);
    SCPISettings write_config();
private slots:

private:
    void initializeComboboxes();

    Ui::fscpi_settings *ui;

    //bool _reconfig = false;
};

template<typename F>
void safe_device_call(QWidget *parent, F &&f) {
    try {
        f();
    } catch (const ScpiDevice::ScpiDeviceError &err) {
        QMessageBox::warning(parent, "Ошибка связи с устройством", err.what());
    } catch (const std::exception &err) {
        QMessageBox::warning(parent, "Неизвестная ошибка", err.what());
    }
}

#define SAFE_CALL_DEVICE(parent, expr) \
safe_device_call(parent, [&]{ expr; })


#endif // SCPISETTINGSUI_H
