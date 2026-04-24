#ifndef SCPISETTINGS_H
#define SCPISETTINGS_H

#include <QWidget>

#include "core-io-lib-global.h"

namespace Ui { class fscpi_settings; }

struct SCPISettings {
    bool SCPI_ENABLE = true;
    QString SCPI_TYPE = "TCP";
    QString SCPI_IP = "192.168.1.1";
    int SCPI_PORT = 502;
    QString SCPI_SERIAL_PORT = "COM1";
    int SCPI_BAUDRATE = 115200;
    int SCPI_DATA_BITS = 8;
    int SCPI_STOP_BITS = 1;
    QString SCPI_PARITY = "NoParity";
    QString SCPI_STATE = "";
};

class CORE_IO_LIB_EXPORT SCPISettingsUI : public QWidget {
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

#endif // SCPISETTINGS_H
