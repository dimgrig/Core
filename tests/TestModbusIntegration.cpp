#include "TestModbusIntegration.h"

#include <cfloat>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>
#include <QModbusDataUnit>

#include <QCheckBox>
#include <QSpinBox>

#include "Logger.h"
#include "ModbusClient.h"
#include "ModbusDataConverter.h"
#include "ModbusServer.h"
#include "ModbusSettings.h"

#include "BindingManager.h"
#include "BitAwareBindingManager.h"
#include "CheckBoxBinding.h"
#include "DoubleSpinBoxBinding.h"
#include "SpinBoxBinding.h"
#include "LineEditBinding.h"

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
    QTRY_COMPARE_WITH_TIMEOUT(modbusServer->state(), QModbusDevice::ConnectedState, 100);

    // Устанавливаем регистры
    modbusServer->setRegister("input1", true);
    modbusServer->setRegister("input2", false);
    modbusServer->setRegister("input3", true);
    modbusServer->setRegister("input4", false);
    modbusServer->setRegister("input5", true);
    modbusServer->setRegister("input6", false);
    modbusServer->setRegister("input7", true);
    modbusServer->setRegister("input8", false);
    modbusServer->setRegister("input9", true);
    modbusServer->setRegister("input10", false);

    modbusServer->setRegister("Coil1", true);
    modbusServer->setRegister("Coil2", false);
    modbusServer->setRegister("Coil3", true);
    modbusServer->setRegister("Coil4", false);
    modbusServer->setRegister("Coil5", true);
    modbusServer->setRegister("Coil6", false);
    modbusServer->setRegister("Coil7", true);
    modbusServer->setRegister("Coil8", false);
    modbusServer->setRegister("Coil9", true);
    modbusServer->setRegister("Coil10", false);

    modbusServer->setRegister("ir1", 1);
    modbusServer->setRegister("ir2", -2);
    modbusServer->setRegister("ir3", 1000000);
    modbusServer->setRegister("ir4", -1000000);
    modbusServer->setRegister("ir5", 5.5);
    modbusServer->setRegister("ir6", -6.6);
    modbusServer->setRegister("ir7", 7.7);
    modbusServer->setRegister("ir8", -8.8);
    modbusServer->setRegister("ir9", 10000000000);
    modbusServer->setRegister("ir10", -10000000000);
    modbusServer->setRegisterString("InputString", "InputString");

    modbusServer->setRegister("hr1", 1);
    modbusServer->setRegister("hr2", -2);
    modbusServer->setRegister("hr3", 1000000);
    modbusServer->setRegister("hr4", -1000000);
    modbusServer->setRegister("hr5", 5.5);
    modbusServer->setRegister("hr6", -6.6);
    modbusServer->setRegister("hr7", 7.7);
    modbusServer->setRegister("hr8", -8.8);
    modbusServer->setRegister("hr9", 10000000000);
    modbusServer->setRegister("hr10", -10000000000);
    modbusServer->setRegisterString("HoldingString", "HoldingString");

    // qCDebug(logDebug()) << modbusServer->get_mdum().size();
    // for (const auto&[regName, mdu] : modbusServer->get_mdum()) {
    //     qCDebug(logDebug()) << regName << mdu.registerType() << mdu.startAddress() << mdu.valueCount();
    // }

    // --- Клиент (наш ModbusClient) ---
    auto modbusClient = std::make_shared<ModbusClient>(nullptr, config, modbusSettings);
    QTest::qWait(100); // Даём клиенту время запуститься
    QSignalSpy readSpy(modbusClient.get(), &ModbusClient::read_ready);
    QSignalSpy stateSpy(modbusClient.get(), &ModbusClient::on_state_changed);

    modbusClient->connectToHost();
    QTRY_COMPARE_WITH_TIMEOUT(modbusClient->state(), QModbusDevice::ConnectedState, 100);

    //--- Тест 1: чтение начальных значений ---
    {
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 39));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 39));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
    modbusServer->setRegister("input1", false);
    modbusServer->setRegister("input2", true);
    modbusServer->setRegister("input3", false);
    modbusServer->setRegister("input4", true);
    modbusServer->setRegister("input5", false);
    modbusServer->setRegister("input6", true);
    modbusServer->setRegister("input7", false);
    modbusServer->setRegister("input8", true);
    modbusServer->setRegister("input9", false);
    modbusServer->setRegister("input10", true);

    modbusServer->setRegister("Coil1", false);
    modbusServer->setRegister("Coil2", true);
    modbusServer->setRegister("Coil3", false);
    modbusServer->setRegister("Coil4", true);
    modbusServer->setRegister("Coil5", false);
    modbusServer->setRegister("Coil6", true);
    modbusServer->setRegister("Coil7", false);
    modbusServer->setRegister("Coil8", true);
    modbusServer->setRegister("Coil9", false);
    modbusServer->setRegister("Coil10", true);

    modbusServer->setRegister("ir1", 2);
    modbusServer->setRegister("ir2", -3);
    modbusServer->setRegister("ir3", 1000001);
    modbusServer->setRegister("ir4", -1000001);
    modbusServer->setRegister("ir5", 5.6);
    modbusServer->setRegister("ir6", -6.7);
    modbusServer->setRegister("ir7", 7.8);
    modbusServer->setRegister("ir8", -8.9);
    modbusServer->setRegister("ir9", 10000000001);
    modbusServer->setRegister("ir10", -10000000001);
    modbusServer->setRegisterString("InputString", "InputString123");

    modbusServer->setRegister("hr1", 2);
    modbusServer->setRegister("hr2", -3);
    modbusServer->setRegister("hr3", 1000001);
    modbusServer->setRegister("hr4", -1000001);
    modbusServer->setRegister("hr5", 5.6);
    modbusServer->setRegister("hr6", -6.7);
    modbusServer->setRegister("hr7", 7.8);
    modbusServer->setRegister("hr8", -8.9);
    modbusServer->setRegister("hr9", 10000000001);
    modbusServer->setRegister("hr10", -10000000001);
    modbusServer->setRegisterString("HoldingString", "HoldingString123");

    {
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 42));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 42));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
    modbusClient->setRegister("input1", true);
    modbusClient->setRegister("input2", false);
    modbusClient->setRegister("input3", true);
    modbusClient->setRegister("input4", false);
    modbusClient->setRegister("input5", true);
    modbusClient->setRegister("input6", false);
    modbusClient->setRegister("input7", true);
    modbusClient->setRegister("input8", false);
    modbusClient->setRegister("input9", true);
    modbusClient->setRegister("input10", false);

    modbusClient->setRegister("Coil1", true);
    modbusClient->setRegister("Coil2", false);
    modbusClient->setRegister("Coil3", true);
    modbusClient->setRegister("Coil4", false);
    modbusClient->setRegister("Coil5", true);
    modbusClient->setRegister("Coil6", false);
    modbusClient->setRegister("Coil7", true);
    modbusClient->setRegister("Coil8", false);
    modbusClient->setRegister("Coil9", true);
    modbusClient->setRegister("Coil10", false);

    modbusClient->setRegister("ir1", 1);
    modbusClient->setRegister("ir2", -2);
    modbusClient->setRegister("ir3", 1000000);
    modbusClient->setRegister("ir4", -1000000);
    modbusClient->setRegister("ir5", 5.5);
    modbusClient->setRegister("ir6", -6.6);
    modbusClient->setRegister("ir7", 7.7);
    modbusClient->setRegister("ir8", -8.8);
    modbusClient->setRegister("ir9", 10000000000);
    modbusClient->setRegister("ir10", -10000000000);
    modbusClient->setRegisterString("InputString", "InputString456");

    modbusClient->setRegister("hr1", 1);
    modbusClient->setRegister("hr2", -2);
    modbusClient->setRegister("hr3", 1000000);
    modbusClient->setRegister("hr4", -1000000);
    modbusClient->setRegister("hr5", 5.5);
    modbusClient->setRegister("hr6", -6.6);
    modbusClient->setRegister("hr7", 7.7);
    modbusClient->setRegister("hr8", -8.8);
    modbusClient->setRegister("hr9", 10000000000);
    modbusClient->setRegister("hr10", -10000000000);
    modbusClient->setRegisterString("HoldingString", "HoldingString456");

    {
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 42));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 39));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
    auto modbusClient = std::make_shared<ModbusClient>();
    BitAwareBindingManager manager(modbusClient);
    auto binding = std::make_shared<TestRegisterBinding>("testReg");
    manager.addBinding(binding);

    auto bindings = manager.get_bindings("testReg");
    QCOMPARE(bindings.size(), 1);
}

void TestModbusIntegration::testProcessModbusDataUpdatesBindings() {
    auto modbusClient = std::make_shared<ModbusClient>();

    // Настроим карту регистров клиента
    QString regName = "testReg";
    auto mdu = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 2);
    QString dataType = "Int32";
    modbusClient->add_mdu_reg_data_type(regName, mdu, dataType);

    BitAwareBindingManager bindingManager(modbusClient);
    auto binding = std::make_shared<TestRegisterBinding>(regName);
    bindingManager.addBinding(binding);

    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, 0, 2);
    unit.setValue(0, 0x1234);
    unit.setValue(1, 0x5678);

    bindingManager.processModbusData(unit);

    QCOMPARE(binding->lastData.size(), 2);
    QCOMPARE(binding->lastData[0], 0x1234);
    QCOMPARE(binding->lastData[1], 0x5678);
}

void TestModbusIntegration::testRegisterWidgetsFromContainer() {
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

    BitAwareBindingManager bindingManager(modbusClient);
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

    BitAwareBindingManager bindingManager(modbusClient);
    auto binding = std::make_shared<TestRegisterBinding>("hr4");
    bindingManager.addBinding(binding);

    // Симулируем изменение значения через UI
    binding->simulateUserChange({0x1111, 0x2222});

    modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 4, 2));
    QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
    QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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

    BitAwareBindingManager bindingManager(modbusClient);
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
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::Coils, 0, 1));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.valueCount(), 1);
        QCOMPARE(unit.value(0), 1);
    }

    {
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 4, 2));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
        QCOMPARE(unit.startAddress(), 4);
        QCOMPARE(unit.valueCount(), 2);
        // QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(0));
        // QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters(-1000000, "hr4", "int", BigEndian).value(1));
    }

    {
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 10, 4));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
        QCOMPARE(unit.startAddress(), 10);
        QCOMPARE(unit.valueCount(), 4);
        QCOMPARE(unit.value(0), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(0));
        QCOMPARE(unit.value(1), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(1));
        QCOMPARE(unit.value(2), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(2));
        QCOMPARE(unit.value(3), ModbusDataConverter::variantToRegisters(7.7, "hr7", "double", BigEndian).value(3));
    }

    {
        modbusClient->on_read(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 26, 4));
        QTRY_COMPARE_WITH_TIMEOUT(readSpy.count(), 1, 2000);
        QModbusDataUnit unit = readSpy.takeFirst().at(0).value<QModbusDataUnit>();
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
