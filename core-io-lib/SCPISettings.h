#ifndef SCPISETTINGS_H
#define SCPISETTINGS_H

#include <QString>

#include "core-io-lib-global.h"

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

class SCPISettingsUI;

#endif // SCPISETTINGS_H
