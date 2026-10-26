#include "TestModbusIntegration.h"

#include <cfloat>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>
#include <QModbusDataUnit>

#include <QCheckBox>
#include <QSpinBox>

#include <core-lib/Logger.h>
#include <core-modbus-lib/ModbusClient.h>
#include <core-modbus-lib/ModbusDataConverter.h>
#include <core-modbus-lib/ModbusServer.h>
#include <core-modbus-lib/ModbusSettings.h>

#include <core-modbus-lib/bindings/BindingManager.h>
#include <core-modbus-lib/bindings/BitAwareBindingManager.h>
#include <core-modbus-lib/bindings/CheckBoxBinding.h>
#include <core-modbus-lib/bindings/DoubleSpinBoxBinding.h>
#include <core-modbus-lib/bindings/SpinBoxBinding.h>
#include <core-modbus-lib/bindings/LineEditBinding.h>

////QTEST_MAIN(TestModbusIntegration) //need when no main.cpp with main function

Q_DECLARE_METATYPE(QModbusDataUnit)

void TestModbusIntegration::initTestCase()
{
    qRegisterMetaType<QModbusDataUnit>("QModbusDataUnit");
}

void TestModbusIntegration::testClientServerCommunication()
{
    QString config = ":/json/json/modbus_10.json";

    // --- Настройки ---
    ModbusSettings modbusSettings;
    modbusSettings.MODBUS_ENABLE = true;
    modbusSettings.MODBUS_TYPE = "TCP";
    modbusSettings.MODBUS_IP = "127.0.0.1";
    modbusSettings.MODBUS_PORT = 1502; // свободный порт
    modbusSettings.MODBUS_ADDRESS = 1;
    modbusSettings.MODBUS_POLL_TIMER = 100;

    // --- Создаём и настраиваем сервер
    auto modbusServer = std::make_shared<ModbusServer>(nullptr, config, modbusSettings);
    QTest::qWait(500); // Даём серверу время запуститься

    // Запускаем сервер
    modbusServer->connectToHost(); //QVERIFY2(modbusServer->connectToHost(), "Failed to start server");
    QTRY_COMPARE_WITH_TIMEOUT(modbusServer->state(), QModbusDevice::ConnectedState, 500);

    // Устанавливаем регистры
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input1", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input2", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input3", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input4", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input5", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input6", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input7", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input8", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input9", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input10", false);

    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil1", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil2", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil3", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil4", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil5", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil6", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil7", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil8", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil9", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil10", false);

    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir1", 1);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir2", -2);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir3", 1000000);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir4", -1000000);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir5", 5.5);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir6", -6.6);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir7", 7.7);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir8", -8.8);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir9", 10000000000);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir10", -10000000000);
    modbusServer->setRegisterString(modbusSettings.MODBUS_ADDRESS, "InputString", "InputString");

    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr1", 1);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr2", -2);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr3", 1000000);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr4", -1000000);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr5", 5.5);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr6", -6.6);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr7", 7.7);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr8", -8.8);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr9", 10000000000);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr10", -10000000000);
    modbusServer->setRegisterString(modbusSettings.MODBUS_ADDRESS, "HoldingString", "HoldingString");

    // qCDebug(logDebug()) << modbusServer->get_mdum().size();
    // for (const auto&[regName, mdu] : modbusServer->get_mdum()) {
    //     qCDebug(logDebug()) << regName << mdu.registerType() << mdu.startAddress() << mdu.valueCount();
    // }

    // --- Клиент (наш ModbusClient) ---
    auto modbusClient = std::make_shared<ModbusClient>(nullptr, config, modbusSettings);
    QTest::qWait(500); // Даём клиенту время запуститься
    QSignalSpy readSpy(modbusClient.get(), &ModbusClient::read_ready);
    QSignalSpy stateSpy(modbusClient.get(), &ModbusClient::on_state_changed);

    // connect(modbusClient.get(), &ModbusClient::read_ready, [=](int serverAddress, QModbusDataUnit mdu){
    //     qCDebug(logDebug) << serverAddress << mdu.valueCount() << mdu.values();
    // });

    modbusClient->connectToHost();
    QTRY_COMPARE_WITH_TIMEOUT(modbusClient->state(), QModbusDevice::ConnectedState, 100);
    QTest::qWait(100);               // Небольшая пауза для завершения инициализации
    modbusClient->stopPolling();     // Останавливаем фоновый опрос
    modbusClient->clearQueue();
    QTest::qWait(50);                // даём время завершиться текущему запросу, если он был
    readSpy.clear();                 // Очищаем возможные сигналы

    //--- Тест 1: чтение начальных значений ---
    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        //qCDebug(logDebug) << serverAddress << unit.valueCount() << unit.values();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.value(0), 1);
        QCOMPARE(unit.value(1), 0);
        QCOMPARE(unit.value(2), 1);
        QCOMPARE(unit.value(3), 0);
        QCOMPARE(unit.value(4), 1);
        QCOMPARE(unit.value(5), 0);
        QCOMPARE(unit.value(6), 1);
        QCOMPARE(unit.value(7), 0);
        QCOMPARE(unit.value(8), 1);
        QCOMPARE(unit.value(9), 0);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        //qCDebug(logDebug) << serverAddress << unit.valueCount() << unit.values();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.value(0), 1);
        QCOMPARE(unit.value(1), 0);
        QCOMPARE(unit.value(2), 1);
        QCOMPARE(unit.value(3), 0);
        QCOMPARE(unit.value(4), 1);
        QCOMPARE(unit.value(5), 0);
        QCOMPARE(unit.value(6), 1);
        QCOMPARE(unit.value(7), 0);
        QCOMPARE(unit.value(8), 1);
        QCOMPARE(unit.value(9), 0);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 39));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        //qCDebug(logDebug) << serverAddress << unit.valueCount() << unit.values();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 39);
        QCOMPARE(unit.value(0), 1);
        QCOMPARE(unit.value(1), (quint16)-2);
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(1000000, "ir3", "uint", BigEndian).value(0));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(1000000, "ir3", "uint", BigEndian).value(1));
        QCOMPARE(unit.value(4), ModbusDataConverter::variantToRegisters(-1000000, "ir4", "int", BigEndian).value(0));
        QCOMPARE(unit.value(5), ModbusDataConverter::variantToRegisters(-1000000, "ir4", "int", BigEndian).value(1));
        QCOMPARE(unit.value(6), ModbusDataConverter::variantToRegisters(5.5, "ir5", "float", BigEndian).value(0));
        QCOMPARE(unit.value(7), ModbusDataConverter::variantToRegisters(5.5, "ir5", "float", BigEndian).value(1));
        QCOMPARE(unit.value(8), ModbusDataConverter::variantToRegisters(-6.6, "ir6", "float", BigEndian).value(0));
        QCOMPARE(unit.value(9), ModbusDataConverter::variantToRegisters(-6.6, "ir6", "float", BigEndian).value(1));
        QCOMPARE(unit.value(10), ModbusDataConverter::variantToRegisters(7.7, "ir7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(11), ModbusDataConverter::variantToRegisters(7.7, "ir7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(12), ModbusDataConverter::variantToRegisters(7.7, "ir7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(13), ModbusDataConverter::variantToRegisters(7.7, "ir7", "double", BigEndian).value(3));
        QCOMPARE(unit.value(14), ModbusDataConverter::variantToRegisters(-8.8, "ir8", "double", BigEndian).value(0));
        QCOMPARE(unit.value(15), ModbusDataConverter::variantToRegisters(-8.8, "ir8", "double", BigEndian).value(1));
        QCOMPARE(unit.value(16), ModbusDataConverter::variantToRegisters(-8.8, "ir8", "double", BigEndian).value(2));
        QCOMPARE(unit.value(17), ModbusDataConverter::variantToRegisters(-8.8, "ir8", "double", BigEndian).value(3));
        QCOMPARE(unit.value(18), ModbusDataConverter::variantToRegisters(10000000000ULL, "ir9", "ulong", BigEndian).value(0));
        QCOMPARE(unit.value(19), ModbusDataConverter::variantToRegisters(10000000000ULL, "ir9", "ulong", BigEndian).value(1));
        QCOMPARE(unit.value(20), ModbusDataConverter::variantToRegisters(10000000000ULL, "ir9", "ulong", BigEndian).value(2));
        QCOMPARE(unit.value(21), ModbusDataConverter::variantToRegisters(10000000000ULL, "ir9", "ulong", BigEndian).value(3));
        QCOMPARE(unit.value(22), ModbusDataConverter::variantToRegisters(-10000000000LL, "ir10", "long", BigEndian).value(0));
        QCOMPARE(unit.value(23), ModbusDataConverter::variantToRegisters(-10000000000LL, "ir10", "long", BigEndian).value(1));
        QCOMPARE(unit.value(24), ModbusDataConverter::variantToRegisters(-10000000000LL, "ir10", "long", BigEndian).value(2));
        QCOMPARE(unit.value(25), ModbusDataConverter::variantToRegisters(-10000000000LL, "ir10", "long", BigEndian).value(3));
        QCOMPARE(unit.value(26), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(0));
        QCOMPARE(unit.value(27), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(1));
        QCOMPARE(unit.value(28), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(2));
        QCOMPARE(unit.value(29), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(3));
        QCOMPARE(unit.value(30), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(4));
        QCOMPARE(unit.value(31), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(5));
        QCOMPARE(unit.value(32), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(6));
        QCOMPARE(unit.value(33), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(7));
        QCOMPARE(unit.value(34), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(8));
        QCOMPARE(unit.value(35), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(9));
        QCOMPARE(unit.value(36), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(10));
        QCOMPARE(unit.value(37), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(11));
        QCOMPARE(unit.value(38), ModbusDataConverter::variantToRegisters("InputString", "InputString", "string", BigEndian).value(12));
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 39));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        //qCDebug(logDebug) << serverAddress << unit.valueCount() << unit.values();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 39);
        QCOMPARE(unit.value(0), 1);
        QCOMPARE(unit.value(1), (quint16)-2);
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(1000000, "hr3", "uint", BigEndian).value(0));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(1000000, "hr3", "uint", BigEndian).value(1));
        QCOMPARE(unit.value(4), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(0));
        QCOMPARE(unit.value(5), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(1));
        QCOMPARE(unit.value(6), ModbusDataConverter::variantToRegisters(5.5, "hr5", "float", BigEndian).value(0));
        QCOMPARE(unit.value(7), ModbusDataConverter::variantToRegisters(5.5, "hr5", "float", BigEndian).value(1));
        QCOMPARE(unit.value(8), ModbusDataConverter::variantToRegisters(-6.6, "hr6", "float", BigEndian).value(0));
        QCOMPARE(unit.value(9), ModbusDataConverter::variantToRegisters(-6.6, "hr6", "float", BigEndian).value(1));
        QCOMPARE(unit.value(10), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(11), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(12), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(13), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(3));
        QCOMPARE(unit.value(14), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(0));
        QCOMPARE(unit.value(15), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(1));
        QCOMPARE(unit.value(16), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(2));
        QCOMPARE(unit.value(17), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(3));
        QCOMPARE(unit.value(18), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(0));
        QCOMPARE(unit.value(19), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(1));
        QCOMPARE(unit.value(20), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(2));
        QCOMPARE(unit.value(21), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(3));
        QCOMPARE(unit.value(22), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(0));
        QCOMPARE(unit.value(23), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(1));
        QCOMPARE(unit.value(24), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(2));
        QCOMPARE(unit.value(25), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(3));
        QCOMPARE(unit.value(26), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(0));
        QCOMPARE(unit.value(27), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(1));
        QCOMPARE(unit.value(28), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(2));
        QCOMPARE(unit.value(29), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(3));
        QCOMPARE(unit.value(30), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(4));
        QCOMPARE(unit.value(31), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(5));
        QCOMPARE(unit.value(32), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(6));
        QCOMPARE(unit.value(33), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(7));
        QCOMPARE(unit.value(34), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(8));
        QCOMPARE(unit.value(35), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(9));
        QCOMPARE(unit.value(36), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(10));
        QCOMPARE(unit.value(37), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(11));
        QCOMPARE(unit.value(38), ModbusDataConverter::variantToRegisters("HoldingString", "hr11", "string", BigEndian).value(12));
    }

    // --- Тест 2: обновление сервером ---
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input1", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input2", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input3", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input4", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input5", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input6", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input7", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input8", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input9", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "input10", true);

    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil1", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil2", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil3", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil4", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil5", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil6", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil7", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil8", true);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil9", false);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil10", true);

    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir1", 2);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir2", -3);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir3", 1000001);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir4", -1000001);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir5", 5.6);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir6", -6.7);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir7", 7.8);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir8", -8.9);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir9", 10000000001);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "ir10", -10000000001);
    modbusServer->setRegisterString(modbusSettings.MODBUS_ADDRESS, "InputString", "InputString123");

    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr1", 2);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr2", -3);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr3", 1000001);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr4", -1000001);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr5", 5.6);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr6", -6.7);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr7", 7.8);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr8", -8.9);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr9", 10000000001);
    modbusServer->setRegister(modbusSettings.MODBUS_ADDRESS, "hr10", -10000000001);
    modbusServer->setRegisterString(modbusSettings.MODBUS_ADDRESS, "HoldingString", "HoldingString123");

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.value(0), 0);
        QCOMPARE(unit.value(1), 1);
        QCOMPARE(unit.value(2), 0);
        QCOMPARE(unit.value(3), 1);
        QCOMPARE(unit.value(4), 0);
        QCOMPARE(unit.value(5), 1);
        QCOMPARE(unit.value(6), 0);
        QCOMPARE(unit.value(7), 1);
        QCOMPARE(unit.value(8), 0);
        QCOMPARE(unit.value(9), 1);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.value(0), 0);
        QCOMPARE(unit.value(1), 1);
        QCOMPARE(unit.value(2), 0);
        QCOMPARE(unit.value(3), 1);
        QCOMPARE(unit.value(4), 0);
        QCOMPARE(unit.value(5), 1);
        QCOMPARE(unit.value(6), 0);
        QCOMPARE(unit.value(7), 1);
        QCOMPARE(unit.value(8), 0);
        QCOMPARE(unit.value(9), 1);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 42));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 42);
        QCOMPARE(unit.value(0), 2);
        QCOMPARE(unit.value(1), (quint16)-3);
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(1000001, "ir3", "uint", BigEndian).value(0));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(1000001, "ir3", "uint", BigEndian).value(1));
        QCOMPARE(unit.value(4), ModbusDataConverter::variantToRegisters(-1000001, "ir4", "int", BigEndian).value(0));
        QCOMPARE(unit.value(5), ModbusDataConverter::variantToRegisters(-1000001, "ir4", "int", BigEndian).value(1));
        QCOMPARE(unit.value(6), ModbusDataConverter::variantToRegisters(5.6, "ir5", "float", BigEndian).value(0));
        QCOMPARE(unit.value(7), ModbusDataConverter::variantToRegisters(5.6, "ir5", "float", BigEndian).value(1));
        QCOMPARE(unit.value(8), ModbusDataConverter::variantToRegisters(-6.7, "ir6", "float", BigEndian).value(0));
        QCOMPARE(unit.value(9), ModbusDataConverter::variantToRegisters(-6.7, "ir6", "float", BigEndian).value(1));
        QCOMPARE(unit.value(10), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(11), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(12), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(13), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(3));
        QCOMPARE(unit.value(14), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(0));
        QCOMPARE(unit.value(15), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(1));
        QCOMPARE(unit.value(16), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(2));
        QCOMPARE(unit.value(17), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(3));
        QCOMPARE(unit.value(18), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(0));
        QCOMPARE(unit.value(19), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(1));
        QCOMPARE(unit.value(20), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(2));
        QCOMPARE(unit.value(21), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(3));
        QCOMPARE(unit.value(22), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(0));
        QCOMPARE(unit.value(23), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(1));
        QCOMPARE(unit.value(24), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(2));
        QCOMPARE(unit.value(25), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(3));
        QCOMPARE(unit.value(26), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(0));
        QCOMPARE(unit.value(27), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(1));
        QCOMPARE(unit.value(28), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(2));
        QCOMPARE(unit.value(29), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(3));
        QCOMPARE(unit.value(30), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(4));
        QCOMPARE(unit.value(31), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(5));
        QCOMPARE(unit.value(32), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(6));
        QCOMPARE(unit.value(33), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(7));
        QCOMPARE(unit.value(34), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(8));
        QCOMPARE(unit.value(35), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(9));
        QCOMPARE(unit.value(36), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(10));
        QCOMPARE(unit.value(37), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(11));
        QCOMPARE(unit.value(38), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(12));
        QCOMPARE(unit.value(39), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(13));
        QCOMPARE(unit.value(40), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(14));
        QCOMPARE(unit.value(41), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(15));
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 42));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 42);
        QCOMPARE(unit.value(0), 2);
        QCOMPARE(unit.value(1), (quint16)-3);
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(1000001, "hr3", "uint", BigEndian).value(0));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(1000001, "hr3", "uint", BigEndian).value(1));
        QCOMPARE(unit.value(4), ModbusDataConverter::variantToRegisters(-1000001, "hr4", "int", BigEndian).value(0));
        QCOMPARE(unit.value(5), ModbusDataConverter::variantToRegisters(-1000001, "hr4", "int", BigEndian).value(1));
        QCOMPARE(unit.value(6), ModbusDataConverter::variantToRegisters(5.6, "hr5", "float", BigEndian).value(0));
        QCOMPARE(unit.value(7), ModbusDataConverter::variantToRegisters(5.6, "hr5", "float", BigEndian).value(1));
        QCOMPARE(unit.value(8), ModbusDataConverter::variantToRegisters(-6.7, "hr6", "float", BigEndian).value(0));
        QCOMPARE(unit.value(9), ModbusDataConverter::variantToRegisters(-6.7, "hr6", "float", BigEndian).value(1));
        QCOMPARE(unit.value(10), ModbusDataConverter::variantToRegisters(7.8, "hr7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(11), ModbusDataConverter::variantToRegisters(7.8, "hr7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(12), ModbusDataConverter::variantToRegisters(7.8, "hr7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(13), ModbusDataConverter::variantToRegisters(7.8, "hr7", "double", BigEndian).value(3));
        QCOMPARE(unit.value(14), ModbusDataConverter::variantToRegisters(-8.9, "hr8", "double", BigEndian).value(0));
        QCOMPARE(unit.value(15), ModbusDataConverter::variantToRegisters(-8.9, "hr8", "double", BigEndian).value(1));
        QCOMPARE(unit.value(16), ModbusDataConverter::variantToRegisters(-8.9, "hr8", "double", BigEndian).value(2));
        QCOMPARE(unit.value(17), ModbusDataConverter::variantToRegisters(-8.9, "hr8", "double", BigEndian).value(3));
        QCOMPARE(unit.value(18), ModbusDataConverter::variantToRegisters(10000000001ULL, "hr9", "ulong", BigEndian).value(0));
        QCOMPARE(unit.value(19), ModbusDataConverter::variantToRegisters(10000000001ULL, "hr9", "ulong", BigEndian).value(1));
        QCOMPARE(unit.value(20), ModbusDataConverter::variantToRegisters(10000000001ULL, "hr9", "ulong", BigEndian).value(2));
        QCOMPARE(unit.value(21), ModbusDataConverter::variantToRegisters(10000000001ULL, "hr9", "ulong", BigEndian).value(3));
        QCOMPARE(unit.value(22), ModbusDataConverter::variantToRegisters(-10000000001LL, "hr10", "long", BigEndian).value(0));
        QCOMPARE(unit.value(23), ModbusDataConverter::variantToRegisters(-10000000001LL, "hr10", "long", BigEndian).value(1));
        QCOMPARE(unit.value(24), ModbusDataConverter::variantToRegisters(-10000000001LL, "hr10", "long", BigEndian).value(2));
        QCOMPARE(unit.value(25), ModbusDataConverter::variantToRegisters(-10000000001LL, "hr10", "long", BigEndian).value(3));
        QCOMPARE(unit.value(26), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(0));
        QCOMPARE(unit.value(27), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(1));
        QCOMPARE(unit.value(28), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(2));
        QCOMPARE(unit.value(29), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(3));
        QCOMPARE(unit.value(30), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(4));
        QCOMPARE(unit.value(31), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(5));
        QCOMPARE(unit.value(32), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(6));
        QCOMPARE(unit.value(33), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(7));
        QCOMPARE(unit.value(34), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(8));
        QCOMPARE(unit.value(35), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(9));
        QCOMPARE(unit.value(36), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(10));
        QCOMPARE(unit.value(37), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(11));
        QCOMPARE(unit.value(38), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(12));
        QCOMPARE(unit.value(39), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(13));
        QCOMPARE(unit.value(40), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(14));
        QCOMPARE(unit.value(41), ModbusDataConverter::variantToRegisters("HoldingString123", "hr11", "string", BigEndian).value(15));
    }

    // --- Тест 3: запись клиентом ---
    // no write to input
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input1", true);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input2", false);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input3", true);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input4", false);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input5", true);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input6", false);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input7", true);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input8", false);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input9", true);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "input10", false);

    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil1", true);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil2", false);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil3", true);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil4", false);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil5", true);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil6", false);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil7", true);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil8", false);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil9", true);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "Coil10", false);

    // no write to input
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir1", 1);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir2", -2);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir3", 1000000);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir4", -1000000);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir5", 5.5);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir6", -6.6);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir7", 7.7);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir8", -8.8);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir9", 10000000000);
    // modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "ir10", -10000000000);
    // modbusClient->setRegisterString(modbusSettings.MODBUS_ADDRESS, "InputString", "InputString456");

    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr1", 1);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr2", -2);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr3", 1000000);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr4", -1000000);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr5", 5.5);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr6", -6.6);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr7", 7.7);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr8", -8.8);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr9", 10000000000);
    modbusClient->setRegister(modbusSettings.MODBUS_ADDRESS, "hr10", -10000000000);
    modbusClient->setRegisterString(modbusSettings.MODBUS_ADDRESS, "HoldingString", "HoldingString456");

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.value(0), 0);
        QCOMPARE(unit.value(1), 1);
        QCOMPARE(unit.value(2), 0);
        QCOMPARE(unit.value(3), 1);
        QCOMPARE(unit.value(4), 0);
        QCOMPARE(unit.value(5), 1);
        QCOMPARE(unit.value(6), 0);
        QCOMPARE(unit.value(7), 1);
        QCOMPARE(unit.value(8), 0);
        QCOMPARE(unit.value(9), 1);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.value(0), 1);
        QCOMPARE(unit.value(1), 0);
        QCOMPARE(unit.value(2), 1);
        QCOMPARE(unit.value(3), 0);
        QCOMPARE(unit.value(4), 1);
        QCOMPARE(unit.value(5), 0);
        QCOMPARE(unit.value(6), 1);
        QCOMPARE(unit.value(7), 0);
        QCOMPARE(unit.value(8), 1);
        QCOMPARE(unit.value(9), 0);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 42));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 42);
        QCOMPARE(unit.value(0), 2);
        QCOMPARE(unit.value(1), (quint16)-3);
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(1000001, "ir3", "uint", BigEndian).value(0));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(1000001, "ir3", "uint", BigEndian).value(1));
        QCOMPARE(unit.value(4), ModbusDataConverter::variantToRegisters(-1000001, "ir4", "int", BigEndian).value(0));
        QCOMPARE(unit.value(5), ModbusDataConverter::variantToRegisters(-1000001, "ir4", "int", BigEndian).value(1));
        QCOMPARE(unit.value(6), ModbusDataConverter::variantToRegisters(5.6, "ir5", "float", BigEndian).value(0));
        QCOMPARE(unit.value(7), ModbusDataConverter::variantToRegisters(5.6, "ir5", "float", BigEndian).value(1));
        QCOMPARE(unit.value(8), ModbusDataConverter::variantToRegisters(-6.7, "ir6", "float", BigEndian).value(0));
        QCOMPARE(unit.value(9), ModbusDataConverter::variantToRegisters(-6.7, "ir6", "float", BigEndian).value(1));
        QCOMPARE(unit.value(10), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(11), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(12), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(13), ModbusDataConverter::variantToRegisters(7.8, "ir7", "double", BigEndian).value(3));
        QCOMPARE(unit.value(14), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(0));
        QCOMPARE(unit.value(15), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(1));
        QCOMPARE(unit.value(16), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(2));
        QCOMPARE(unit.value(17), ModbusDataConverter::variantToRegisters(-8.9, "ir8", "double", BigEndian).value(3));
        QCOMPARE(unit.value(18), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(0));
        QCOMPARE(unit.value(19), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(1));
        QCOMPARE(unit.value(20), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(2));
        QCOMPARE(unit.value(21), ModbusDataConverter::variantToRegisters(10000000001ULL, "ir9", "ulong", BigEndian).value(3));
        QCOMPARE(unit.value(22), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(0));
        QCOMPARE(unit.value(23), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(1));
        QCOMPARE(unit.value(24), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(2));
        QCOMPARE(unit.value(25), ModbusDataConverter::variantToRegisters(-10000000001LL, "ir10", "long", BigEndian).value(3));
        QCOMPARE(unit.value(26), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(0));
        QCOMPARE(unit.value(27), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(1));
        QCOMPARE(unit.value(28), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(2));
        QCOMPARE(unit.value(29), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(3));
        QCOMPARE(unit.value(30), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(4));
        QCOMPARE(unit.value(31), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(5));
        QCOMPARE(unit.value(32), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(6));
        QCOMPARE(unit.value(33), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(7));
        QCOMPARE(unit.value(34), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(8));
        QCOMPARE(unit.value(35), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(9));
        QCOMPARE(unit.value(36), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(10));
        QCOMPARE(unit.value(37), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(11));
        QCOMPARE(unit.value(38), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(12));
        QCOMPARE(unit.value(39), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(13));
        QCOMPARE(unit.value(40), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(14));
        QCOMPARE(unit.value(41), ModbusDataConverter::variantToRegisters("InputString123", "InputString", "string", BigEndian).value(15));
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 39));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 39);
        QCOMPARE(unit.value(0), 1);
        QCOMPARE(unit.value(1), (quint16)-2);
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(1000000, "hr3", "uint", BigEndian).value(0));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(1000000, "hr3", "uint", BigEndian).value(1));
        QCOMPARE(unit.value(4), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(0));
        QCOMPARE(unit.value(5), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(1));
        QCOMPARE(unit.value(6), ModbusDataConverter::variantToRegisters(5.5, "hr5", "float", BigEndian).value(0));
        QCOMPARE(unit.value(7), ModbusDataConverter::variantToRegisters(5.5, "hr5", "float", BigEndian).value(1));
        QCOMPARE(unit.value(8), ModbusDataConverter::variantToRegisters(-6.6, "hr6", "float", BigEndian).value(0));
        QCOMPARE(unit.value(9), ModbusDataConverter::variantToRegisters(-6.6, "hr6", "float", BigEndian).value(1));
        QCOMPARE(unit.value(10), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(11), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(12), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(13), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(3));
        QCOMPARE(unit.value(14), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(0));
        QCOMPARE(unit.value(15), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(1));
        QCOMPARE(unit.value(16), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(2));
        QCOMPARE(unit.value(17), ModbusDataConverter::variantToRegisters(-8.8, "hr8", "double", BigEndian).value(3));
        QCOMPARE(unit.value(18), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(0));
        QCOMPARE(unit.value(19), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(1));
        QCOMPARE(unit.value(20), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(2));
        QCOMPARE(unit.value(21), ModbusDataConverter::variantToRegisters(10000000000ULL, "hr9", "ulong", BigEndian).value(3));
        QCOMPARE(unit.value(22), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(0));
        QCOMPARE(unit.value(23), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(1));
        QCOMPARE(unit.value(24), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(2));
        QCOMPARE(unit.value(25), ModbusDataConverter::variantToRegisters(-10000000000LL, "hr10", "long", BigEndian).value(3));
        QCOMPARE(unit.value(26), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(0));
        QCOMPARE(unit.value(27), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(1));
        QCOMPARE(unit.value(28), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(2));
        QCOMPARE(unit.value(29), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(3));
        QCOMPARE(unit.value(30), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(4));
        QCOMPARE(unit.value(31), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(5));
        QCOMPARE(unit.value(32), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(6));
        QCOMPARE(unit.value(33), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(7));
        QCOMPARE(unit.value(34), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(8));
        QCOMPARE(unit.value(35), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(9));
        QCOMPARE(unit.value(36), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(10));
        QCOMPARE(unit.value(37), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(11));
        QCOMPARE(unit.value(38), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(12));
        QCOMPARE(unit.value(39), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(13));
        QCOMPARE(unit.value(40), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(14));
        QCOMPARE(unit.value(41), ModbusDataConverter::variantToRegisters("HoldingString456", "HoldingString", "string", BigEndian).value(15));
    }

    // Очистка
    modbusClient->disconnectFromHost();
    modbusServer->disconnectFromHost();
}

void TestModbusIntegration::testAddBinding() {
    int serverAddress = 1;
    auto modbusClient = std::make_shared<ModbusClient>();
    BitAwareBindingManager manager(serverAddress, modbusClient);
    auto binding = std::make_shared<TestRegisterBinding>("testReg");
    manager.addBinding(binding);

    auto bindings = manager.get_bindings("testReg");
    QCOMPARE(bindings.size(), 1);
}

void TestModbusIntegration::testProcessModbusDataUpdatesBindings() {
    int serverAddress = 1;
    auto modbusClient = std::make_shared<ModbusClient>();

    // Настроим карту регистров клиента
    QString regName = "testReg";
    auto mdu = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2);
    QString dataType = "Int32";
    modbusClient->add_modbus_params(serverAddress, regName, mdu, dataType);

    BitAwareBindingManager bindingManager(serverAddress, modbusClient);
    auto binding = std::make_shared<TestRegisterBinding>(regName);
    bindingManager.addBinding(binding);

    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, 0, 2);
    unit.setValue(0, 0x1234);
    unit.setValue(1, 0x5678);

    bindingManager.processModbusData(serverAddress, unit);

    QCOMPARE(binding->lastData.size(), 2);
    QCOMPARE(binding->lastData[0], 0x1234);
    QCOMPARE(binding->lastData[1], 0x5678);
}

void TestModbusIntegration::testRegisterWidgetsFromContainer() {
    int serverAddress = 1;

    QWidget *container = new QWidget();

    QCheckBox* cb = new QCheckBox("Check", container);
    cb->setProperty("modbusRegister", "Coil1");
    cb->setProperty("modbusDataType", "Bool");
    cb->setProperty("modbusBitPosition", 0);

    QSpinBox* sb = new QSpinBox(container);
    sb->setRange(-INT_MAX, INT_MAX);
    sb->setProperty("modbusRegister", "hr4");
    sb->setProperty("modbusDataType", "Int32");

    QDoubleSpinBox* dsb = new QDoubleSpinBox(container);
    dsb->setRange(-DBL_MAX, DBL_MAX);
    dsb->setProperty("modbusRegister", "hr7");
    dsb->setProperty("modbusDataType", "Double");

    QLineEdit* le = new QLineEdit(container);
    le->setProperty("modbusRegister", "HoldingString");
    le->setProperty("modbusDataType", "String");

    auto modbusClient = std::make_shared<ModbusClient>();

    BitAwareBindingManager bindingManager(serverAddress, modbusClient);
    bindingManager.registerWidgetsFromContainer(container);

    auto bindings_cb = bindingManager.get_bindings("Coil1");
    auto bindings_sb = bindingManager.get_bindings("hr4");
    auto bindings_dsb = bindingManager.get_bindings("hr7");
    auto bindings_le = bindingManager.get_bindings("HoldingString");
    QCOMPARE(bindings_cb.size(), 1);
    QCOMPARE(bindings_sb.size(), 1);
    QCOMPARE(bindings_dsb.size(), 1);
    QCOMPARE(bindings_le.size(), 1);

    delete container;
}

void TestModbusIntegration::testBindingValueChangedTriggersWrite() {
    QString config = ":/json/json/modbus_10.json";

    // --- Настройки ---
    ModbusSettings modbusSettings;
    modbusSettings.MODBUS_ENABLE = true;
    modbusSettings.MODBUS_TYPE = "TCP";
    modbusSettings.MODBUS_IP = "127.0.0.1";
    modbusSettings.MODBUS_PORT = 1502; // свободный порт
    modbusSettings.MODBUS_ADDRESS = 1;
    modbusSettings.MODBUS_POLL_TIMER = 100;

    // --- Создаём и настраиваем сервер
    auto modbusServer = std::make_shared<ModbusServer>(nullptr, config, modbusSettings);
    QTest::qWait(100); // Даём серверу время запуститься

    // Запускаем сервер
    modbusServer->connectToHost(); // QVERIFY2(modbusServer->connectToHost(),
        // "Failed to start server");
    QTRY_COMPARE_WITH_TIMEOUT(modbusServer->state(), QModbusDevice::ConnectedState, 100);

    // Устанавливаем регистры

    // --- Клиент (наш ModbusClient) ---
    auto modbusClient = std::make_shared<ModbusClient>(nullptr, config, modbusSettings);
    QTest::qWait(100); // Даём клиенту время запуститься
    QSignalSpy readSpy(modbusClient.get(), &ModbusClient::read_ready);
    QSignalSpy stateSpy(modbusClient.get(), &ModbusClient::on_state_changed);

    modbusClient->connectToHost();
    QTRY_COMPARE_WITH_TIMEOUT(modbusClient->state(), QModbusDevice::ConnectedState, 100);
    QTest::qWait(100);               // Небольшая пауза для завершения инициализации
    modbusClient->stopPolling();     // Останавливаем фоновый опрос
    modbusClient->clearQueue();
    QTest::qWait(50);                // даём время завершиться текущему запросу, если он был
    readSpy.clear();                 // Очищаем возможные сигналы

    BitAwareBindingManager bindingManager(modbusSettings.MODBUS_ADDRESS, modbusClient);
    auto binding = std::make_shared<TestRegisterBinding>("hr4");
    bindingManager.addBinding(binding);

    // Симулируем изменение значения через UI
    binding->simulateUserChange({0x1111, 0x2222});

    modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 4, 2));
    QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
    QList<QVariant> args = readSpy.takeFirst();
    int serverAddress = args.at(0).value<int>();
    QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
    QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
    QCOMPARE(unit.startAddress(), 4);
    QCOMPARE(unit.valueCount(), 2);
    QCOMPARE(unit.value(0), 0x1111);
    QCOMPARE(unit.value(1), 0x2222);

    // Очистка
    modbusClient->disconnectFromHost();
    modbusServer->disconnectFromHost();
}

void TestModbusIntegration::testBindingValueChangedTriggersWriteFromContainer() {
    QWidget *container = new QWidget();

    QCheckBox* cb = new QCheckBox("Check", container);
    cb->setProperty("modbusRegister", "Coil1");
    cb->setProperty("modbusDataType", "Bool");
    cb->setProperty("modbusBitPosition", 0);

    QSpinBox* sb = new QSpinBox(container);
    sb->setRange(-INT_MAX, INT_MAX);
    sb->setProperty("modbusRegister", "hr4");
    sb->setProperty("modbusDataType", "Int32");

    QDoubleSpinBox* dsb = new QDoubleSpinBox(container);
    dsb->setRange(-DBL_MAX, DBL_MAX);
    dsb->setProperty("modbusRegister", "hr7");
    dsb->setProperty("modbusDataType", "Double");

    QLineEdit* le = new QLineEdit(container);
    le->setProperty("modbusRegister", "HoldingString");
    le->setProperty("modbusDataType", "String");

    QString config = ":/json/json/modbus_10.json";

    // --- Настройки ---
    ModbusSettings modbusSettings;
    modbusSettings.MODBUS_ENABLE = true;
    modbusSettings.MODBUS_TYPE = "TCP";
    modbusSettings.MODBUS_IP = "127.0.0.1";
    modbusSettings.MODBUS_PORT = 1502; // свободный порт
    modbusSettings.MODBUS_ADDRESS = 1;
    modbusSettings.MODBUS_POLL_TIMER = 100;

    // --- Создаём и настраиваем сервер
    auto modbusServer = std::make_shared<ModbusServer>(nullptr, config, modbusSettings);
    QTest::qWait(100); // Даём серверу время запуститься

    // Запускаем сервер
    modbusServer->connectToHost(); // QVERIFY2(modbusServer->connectToHost(),
    QTRY_COMPARE_WITH_TIMEOUT(modbusServer->state(), QModbusDevice::ConnectedState, 100);

    // Устанавливаем регистры

    // --- Клиент (наш ModbusClient) ---
    auto modbusClient = std::make_shared<ModbusClient>(nullptr, config, modbusSettings);
    QTest::qWait(100); // Даём клиенту время запуститься
    QSignalSpy readSpy(modbusClient.get(), &ModbusClient::read_ready);
    QSignalSpy stateSpy(modbusClient.get(), &ModbusClient::on_state_changed);

    modbusClient->connectToHost();
    QTRY_COMPARE_WITH_TIMEOUT(modbusClient->state(), QModbusDevice::ConnectedState, 100);
    QTest::qWait(100);               // Небольшая пауза для завершения инициализации
    modbusClient->stopPolling();     // Останавливаем фоновый опрос
    modbusClient->clearQueue();
    QTest::qWait(50);                // даём время завершиться текущему запросу, если он был
    readSpy.clear();                 // Очищаем возможные сигналы

    BitAwareBindingManager bindingManager(modbusSettings.MODBUS_ADDRESS, modbusClient);
    bindingManager.registerWidgetsFromContainer(container);

    auto bindings_cb = bindingManager.get_bindings("Coil1");
    auto bindings_sb = bindingManager.get_bindings("hr4");
    auto bindings_dsb = bindingManager.get_bindings("hr7");
    auto bindings_le = bindingManager.get_bindings("HoldingString");
    QCOMPARE(bindings_cb.size(), 1);
    QCOMPARE(bindings_sb.size(), 1);
    QCOMPARE(bindings_dsb.size(), 1);
    QCOMPARE(bindings_le.size(), 1);

    if (auto checkBoxBinding = std::dynamic_pointer_cast<CheckBoxBinding>(bindings_cb.first())) {

    } else {
        QVERIFY(false);
    }

    if (auto spinBoxBinding = std::dynamic_pointer_cast<SpinBoxBinding>(bindings_sb.first())) {

    } else {
        QVERIFY(false);
    }

    if (auto doubleSpinBoxBinding = std::dynamic_pointer_cast<DoubleSpinBoxBinding>(bindings_dsb.first())) {

    } else {
        QVERIFY(false);
    }

    if (auto lineEditBinding = std::dynamic_pointer_cast<LineEditBinding>(bindings_le.first())) {

    } else {
        QVERIFY(false);
    }

    cb->setChecked(true);
    sb->setValue(-1000000);
    dsb->setValue(7.7);
    le->setText("ABCD");
    emit le->editingFinished();

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::Coils, 0, 1));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 1);
        QCOMPARE(unit.value(0), 1);
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 4, 2));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 4);
        QCOMPARE(unit.valueCount(), 2);
        // QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(0));
        // QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(1));
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 10, 4));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 10);
        QCOMPARE(unit.valueCount(), 4);
        QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(1), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(3));
    }

    {
        modbusClient->on_read(modbusSettings.MODBUS_ADDRESS, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 26, 4));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QList<QVariant> args = readSpy.takeFirst();
        int serverAddress = args.at(0).value<int>();
        QModbusDataUnit unit = args.at(1).value<QModbusDataUnit>();
        QCOMPARE(serverAddress, modbusSettings.MODBUS_ADDRESS);
        QCOMPARE(unit.startAddress(), 26);
        QCOMPARE(unit.valueCount(), 4);
        QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters("ABCD", "HoldingString", "string", BigEndian).value(0));
        QCOMPARE(unit.value(1), ModbusDataConverter::variantToRegisters("ABCD", "HoldingString", "string", BigEndian).value(1));
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters("ABCD", "HoldingString", "string", BigEndian).value(2));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters("ABCD", "HoldingString", "string", BigEndian).value(3));
    }

    // Очистка
    modbusClient->disconnectFromHost();
    modbusServer->disconnectFromHost();

    delete container;
}

// ────────────────────────────────────────────────────────────
//  Интеграционный тест scale: Factor ("0.1") + Range ("0;100;0;1")
//  Проверяет весь путь: файл → _reg_scale_m → setRegister →
//  raw в _mdum → registersToVariant на клиенте.
// ────────────────────────────────────────────────────────────
static void runScaleTest(const QString& configPath,
                         quint16 expectedFactorRaw,
                         quint16 expectedRangeRaw)
{
    ModbusSettings s;
    s.MODBUS_ENABLE     = true;
    s.MODBUS_TYPE       = "TCP";
    s.MODBUS_IP         = "127.0.0.1";
    s.MODBUS_PORT       = 1503;
    s.MODBUS_ADDRESS    = 1;
    s.MODBUS_POLL_TIMER = 100;

    // ── Сервер ──────────────────────────────────────────────
    auto server = std::make_shared<ModbusServer>(nullptr, configPath, s);
    QTest::qWait(300);
    server->connectToHost();
    QTRY_COMPARE_WITH_TIMEOUT(server->state(), QModbusDevice::ConnectedState, 1000);

    // ── Клиент ──────────────────────────────────────────────
    auto client = std::make_shared<ModbusClient>(nullptr, configPath, s);
    QTest::qWait(300);
    client->connectToHost();
    QTRY_COMPARE_WITH_TIMEOUT(client->state(), QModbusDevice::ConnectedState, 1000);

    // ── Парсинг scale: сервер ───────────────────────────────
    {
        auto sm = server->get_reg_scale_m();
        QVERIFY2(sm.count(1) == 1, "server: no _reg_scale_m[1]");
        QVERIFY2(sm.at(1).count("hr_num")   == 1, "server: hr_num no scale");
        QVERIFY2(sm.at(1).count("hr_range") == 1, "server: hr_range no scale");

        QVERIFY2(sm.at(1).at("hr_num").isFactor(), "server: hr_num not Factor");
        QCOMPARE(sm.at(1).at("hr_num").factor, 0.1);

        QVERIFY2(sm.at(1).at("hr_range").isRange(), "server: hr_range not Range");
        QCOMPARE(sm.at(1).at("hr_range").srcMin, 0.0);
        QCOMPARE(sm.at(1).at("hr_range").srcMax, 100.0);
        QCOMPARE(sm.at(1).at("hr_range").dstMin, 0.0);
        QCOMPARE(sm.at(1).at("hr_range").dstMax, 1.0);
    }

    // ── Парсинг scale: клиент ───────────────────────────────
    {
        auto cm = client->get_reg_scale_m();
        QVERIFY2(cm.count(1) == 1, "client: no _reg_scale_m[1]");
        QVERIFY2(cm.at(1).at("hr_num").isFactor(),   "client: hr_num not Factor");
        QVERIFY2(cm.at(1).at("hr_range").isRange(),  "client: hr_range not Range");
    }

    // ── Сервер: setRegister применяет scale ────────────────
    // 10.0 / 0.1 = 100 (Factor), 0.5 -> 50 (Range 0..100→0..1)
    server->setRegister(1, "hr_num",   10.0);
    server->setRegister(1, "hr_range", 0.5);
    QTest::qWait(200);

    // На сервере _mdum обновляется setRegister'ом (в отличие от внешнего write).
    {
        auto mdum = server->get_mdum();
        QVERIFY2(mdum.count(1) == 1, "server: no _mdum[1]");
        QVERIFY2(mdum.at(1).count("hr_num")   == 1, "server: no hr_num");
        QVERIFY2(mdum.at(1).count("hr_range") == 1, "server: no hr_range");
        QCOMPARE(mdum.at(1).at("hr_num").value(0),   expectedFactorRaw);
        QCOMPARE(mdum.at(1).at("hr_range").value(0), expectedRangeRaw);
    }

    // ── Клиент читает с сервера raw, применяет scale ───────
    {
        QSignalSpy readSpy(client.get(), &ModbusClient::read_ready);
        client->on_read(1, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2));
        QTRY_VERIFY_WITH_TIMEOUT(readSpy.count() >= 1, 2000);

        bool found = false;
        for (int i = 0; i < readSpy.count(); ++i) {
            QModbusDataUnit unit = readSpy.at(i).at(1).value<QModbusDataUnit>();
            if (unit.registerType() == QModbusDataUnit::HoldingRegisters &&
                unit.startAddress() == 0 && unit.valueCount() >= 2) {
                found = true;

                // Raw значения в ответе — 100 и 50
                QCOMPARE(unit.value(0), expectedFactorRaw);
                QCOMPARE(unit.value(1), expectedRangeRaw);

                // registersToVariant с scale → 10.0 и 0.5
                auto scaleNum = client->get_reg_scale_m().at(1).at("hr_num");
                auto scaleRng = client->get_reg_scale_m().at(1).at("hr_range");
                QVariant numVal = ModbusDataConverter::registersToVariant(
                    { unit.value(0) }, "hr_num", "ushort", BigEndian, scaleNum);
                QVariant rngVal = ModbusDataConverter::registersToVariant(
                    { unit.value(1) }, "hr_range", "ushort", BigEndian, scaleRng);
                ASSERT(std::abs(numVal.toDouble() - 10.0) < 0.01);
                ASSERT(std::abs(rngVal.toDouble() - 0.5)  < 0.01);
                break;
            }
        }
        QVERIFY2(found, "read_ready не пришёл с ожидаемым HR 0..1");
    }

    // ── Клиент: write path — setRegister(20.0) → raw 200 ────
    // (не проверяем _mdum на сервере — он не обновляется при внешнем write;
    //  проверяем raw через read-back)
    {
        client->setRegister(1, "hr_num",   20.0);   // 20.0 / 0.1 = 200
        client->setRegister(1, "hr_range", 0.25);   // 0.25 → raw 25
        QTest::qWait(500);

        QSignalSpy readSpy2(client.get(), &ModbusClient::read_ready);
        client->on_read(1, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2));
        QTRY_VERIFY_WITH_TIMEOUT(readSpy2.count() >= 1, 2000);

        bool found = false;
        for (int i = 0; i < readSpy2.count(); ++i) {
            QModbusDataUnit unit = readSpy2.at(i).at(1).value<QModbusDataUnit>();
            if (unit.registerType() == QModbusDataUnit::HoldingRegisters &&
                unit.startAddress() == 0 && unit.valueCount() >= 2) {
                found = true;
                QCOMPARE(unit.value(0), static_cast<quint16>(200));
                QCOMPARE(unit.value(1), static_cast<quint16>(25));
                break;
            }
        }
        QVERIFY2(found, "read-back после client setRegister не пришёл");
    }

    client->disconnectFromHost();
    server->disconnectFromHost();
    QTest::qWait(100);
}

void TestModbusIntegration::testScaleJsonFactorAndRange()
{
    QTemporaryFile tmp(QDir::tempPath() + "/scale_test_XXXXXX.json");
    QVERIFY(tmp.open());
    tmp.write(R"({
        "mdum": [
            {"RegisterName":"hr_num",   "RegisterType":"HoldingRegister", "Address":0,
             "DataLength":1, "RegisterDataType":"ushort",
             "Access":"ReadWrite", "Scale":0.1},
            {"RegisterName":"hr_range", "RegisterType":"HoldingRegister", "Address":1,
             "DataLength":1, "RegisterDataType":"ushort",
             "Access":"ReadWrite", "Scale":"0;100;0;1"}
        ]
    })");
    tmp.close();

    runScaleTest(tmp.fileName(), 100, 50);
}

void TestModbusIntegration::testScaleCsvFactorAndRange()
{
    QTemporaryFile tmp(QDir::tempPath() + "/scale_test_XXXXXX.csv");
    QVERIFY(tmp.open());
    tmp.write(
        "\"Name\";\"Region\";\"Address\";\"DataTypeDevice\";\"DataTypeScada\";"
        "\"Access\";\"TypeString\";\"LengthString\";\"Scale\";\"Comment\"\n"
        "\"hr_num\";\"HOLDING_REGISTERS\";\"0\";\"WORD\";\"WORD\";"
        "\"ReadWrite\";\"\";\"\";\"0.1\";\"hr_num\"\n"
        "\"hr_range\";\"HOLDING_REGISTERS\";\"1\";\"WORD\";\"WORD\";"
        "\"ReadWrite\";\"\";\"\";\"0;100;0;1\";\"hr_range\"\n"
        );
    tmp.close();

    runScaleTest(tmp.fileName(), 100, 50);
}
