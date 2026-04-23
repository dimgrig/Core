#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include <QButtonGroup>
#include <QLineEdit>
#include <QModbusServer>

#include <QMainWindow>
#include <QModbusDataUnit>
#include <QModbusDevice>

//#include "Config.h"

#include "ModbusBase.h"
#include "ModbusSettings.h"
//#include "rv-lib-global.h"

QT_BEGIN_NAMESPACE

//namespace Ui { class fmodbus_server; }

QT_END_NAMESPACE

class CORE_MODBUS_LIB_EXPORT ModbusServer : public ModbusBase {
Q_OBJECT
public:
    explicit ModbusServer(QWidget *parent = nullptr, QString config = "", ModbusSettings = ModbusSettings());
    ~ModbusServer();

    void updateDeviceMap() override;
    void updatePollingGroups() override;

signals:
    void read_ready(int serverAddress, QString reg_name, QModbusDataUnit mdu);
public slots:
    std::shared_ptr<QModbusDevice> device() const override {
        return std::static_pointer_cast<QModbusDevice>(modbusDevice);
    }

    void on_connect_type(int index) override;
    void on_connect() override;
    void disconnectFromHost() override;

    void setRegister(int serverAddress, const QString &reg_name, double value);
    void setRegisterString(int serverAddress, const QString &reg_name, const QString &value);

    void setRegister(int serverAddress, QModbusDataUnit::RegisterType table, const QString &reg_name, double value);
    void setRegisterString(int serverAddress, QModbusDataUnit::RegisterType table, const QString &reg_name, const QString &value);
private slots:
    void onModbusStateChanged(int state);
    void onModbusErrorOccurred(QModbusDevice::Error error);
    void settings_change();

private:
    void updateRegisters(QModbusDataUnit::RegisterType table, int address, int size);

    //QModbusReply *lastRequest = nullptr;
    std::shared_ptr<QModbusServer> modbusDevice;

    // bool _requestInProgress = false;
    // QTimer *_reconnectTimer = nullptr;
    // int _failedAttempts = 0;
    // const int MAX_FAILED_ATTEMPTS = 3;

    // QQueue<QModbusDataUnit> _requestQueue;
    // bool _processingQueue = false;
    // const int MAX_QUEUE_SIZE = 3;
};

#endif // MODBUSSERVER_H
