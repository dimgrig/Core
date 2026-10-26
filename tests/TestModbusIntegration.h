#ifndef TESTMODBUSINTEGRATION_H
#define TESTMODBUSINTEGRATION_H

#include <QtTest/QtTest>
#include <QSignalSpy>

#include <core-modbus-lib/bindings/RegisterBinding.h>

class TestRegisterBinding : public RegisterBinding {
public:
    TestRegisterBinding(const QString& name, QObject* parent = nullptr)
        : RegisterBinding(name, ReadWrite, BigEndian, parent) {}

    void updateFromRegister(const QVector<quint16>& data) override {
        lastData = data;
        m_cachedData = data;
    }
    QVector<quint16> getRegisterData() const override {
        return m_cachedData;
    }
    bool isValid() const override { return true; }

    // Метод для имитации изменения пользователем
    void simulateUserChange(const QVector<quint16>& data) {
        m_cachedData = data;
        emit valueChanged();
    }

    QVector<quint16> lastData;
    QVector<quint16> m_cachedData;
};

class TestModbusIntegration : public QObject
{
    Q_OBJECT
public:

private slots:
    void initTestCase();

    void testClientServerCommunication();
    void testAddBinding();
    void testProcessModbusDataUpdatesBindings();
    void testRegisterWidgetsFromContainer();
    void testBindingValueChangedTriggersWrite();
    void testBindingValueChangedTriggersWriteFromContainer();
    void testScaleJsonFactorAndRange();
    void testScaleCsvFactorAndRange();
};

#endif // TESTMODBUSINTEGRATION_H
