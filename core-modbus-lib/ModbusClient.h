#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QModbusDevice>
#include <QQueue>
#include <QElapsedTimer>
#include "qmodbusclient.h"
//#include <qt5/QtCore/qchar.h>

//#include "Config.h"

#include "ModbusBase.h"

#include "core-modbus-lib-global.h"

QT_BEGIN_NAMESPACE

class QModbusClient;
class QModbusReply;

//namespace Ui { class fmodbus_client; }

QT_END_NAMESPACE

class CORE_MODBUS_LIB_EXPORT ModbusClient : public ModbusBase {
    Q_OBJECT
public:
    ModbusClient(QWidget *parent = nullptr, QString config = "", ModbusSettings = ModbusSettings());
    ~ModbusClient();

    void updateDeviceMap() override;
    void updatePollingGroups() override;

    // bool isReadable() {return this->_socket->isReadable();};
    // int bytesAvailable() {return this->_socket->bytesAvailable();};
    // bool waitForReadyRead(int msecs = 1000) {return this->_socket->waitForReadyRead(msecs);};
    bool reset() {return false;};

    //bool getServerResponsive() const {return _lastResponsiveState;};
    //bool getPollingCycleActive() const {return _pollingCycleActive;};

    bool isServerResponsive(int serverAddress) const;

    void enqueueReadRequest(int serverAddress, const QModbusDataUnit& mdu, bool isPolling = false);
    void enqueueWriteRequest(int serverAddress, const QModbusDataUnit& mdu, bool highPriority = false);
    void startPolling();
    void stopPolling();
    void clearQueue();

    bool processingQueue() const {return _processingQueue;};

    void addPollingGroup(int serverAddress, const QModbusDataUnit& unit);
    void removePollingGroupsForAddress(int serverAddress);
    void clearPollingGroups();
signals:
    void read_ready(int serverAddress, QModbusDataUnit mdu);

    //void serverHealthChanged(bool responsive);
    void serverHealthChanged(int serverAddress, bool responsive);
public slots:
    std::shared_ptr<QModbusDevice> device() const override {
        return std::static_pointer_cast<QModbusDevice>(modbusDevice);
    }

    void on_connect_type(int index) override;
    void on_connect() override;
    void disconnectFromHost() override;

    void sendSingleRequest(int serverAddress, QModbusDataUnit mdu);

    void on_read(int serverAddress, QModbusDataUnit mdu);
    void on_write(int serverAddress, QModbusDataUnit mdu);
    void on_read(int serverAddress, QModbusDataUnit::RegisterType type, int startAddress, quint16 numberOfEntries);
    void on_write(int serverAddress, QModbusDataUnit::RegisterType type, int startAddress, quint16 numberOfEntries);

    void setRegister(int serverAddress, const QString &reg_name, double value);
    void setRegisterString(int serverAddress, const QString &reg_name, const QString &value);

    void writeMultipleRegisters(int serverAddress, int startAddress, const QVector<quint16>& values);

    QModbusDataUnit getLastReadUnit() const {return _lastReadUnit;};
    QModbusDataUnit getLastWriteUnit()  const {return _lastWriteUnit;};
private slots:
    void onModbusStateChanged(int state);
    void onModbusErrorOccurred(QModbusDevice::Error error);

    void settings_change();
    void on_read_ready();

    void processNextRequest();
    void startPollingCycle();
private:
    struct Request {
        QModbusDataUnit unit;
        int serverAddress;
        bool isWrite;
        bool highPriority;
        bool isPolling; // true, если запрос является частью циклического опроса
    };

    struct PollingGroup {
        int serverAddress;
        QModbusDataUnit unit;
    };

    void setRegister(int serverAddress, QModbusDataUnit::RegisterType table, const QString &reg_name, double value);
    void setRegisterString(int serverAddress, QModbusDataUnit::RegisterType table, const QString &reg_name, const QString &value);
    void scheduleReconnect();

    QModbusReply *lastRequest = nullptr;
    std::shared_ptr<QModbusClient> modbusDevice;

    bool _requestInProgress = false;
    QTimer *_reconnectTimer = nullptr;
    int _failedAttempts = 0;
    const int MAX_FAILED_ATTEMPTS = 3;

    QQueue<Request> _requestQueue;
    bool _processingQueue = false;
    const int MAX_QUEUE_SIZE = 100;

    QTimer *_pollTimer = nullptr;
    std::vector<PollingGroup> _pollingGroups; // группы регистров для периодического чтения
    int _currentPollGroupIndex = 0;
    int _pendingPollCount = 0;
    bool _pollingCycleActive = false;

    const int MAX_FAILED_REQUESTS = 5;
    //int _requestErrorCount = 0;
    int _requestErrorThreshold = MAX_FAILED_REQUESTS; // порог можно сделать настраиваемым
    //bool _lastResponsiveState = true; // для избежания лишних сигналов
    std::map<int, bool> _serverResponsiveState;
    std::map<int, int> _serverRequestErrorCount;

    QModbusDataUnit _lastReadUnit = QModbusDataUnit();
    QModbusDataUnit _lastWriteUnit = QModbusDataUnit();

    int _consecutiveErrorCount = 0;
    QElapsedTimer _lastErrorLogTimer;
    QModbusDevice::Error _lastErrorType = QModbusDevice::NoError;
};

#endif // MODBUSCLIENT_H

