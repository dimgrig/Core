#include "MockModbusClient.h"

MockModbusClient::MockModbusClient(QWidget *parent)
    : ModbusClient(parent, QString(), ModbusSettings())  // пустая конфигурация
{
    // Отключаем все таймеры и очереди реального клиента
    stopPolling();
    clearQueue();
    // Важно: не вызываем connectToHost()
}

void MockModbusClient::sendSingleRequest(int serverAddress, QModbusDataUnit mdu) {
    // Вместо реального запроса возвращаем заданный ответ
    if (m_responses.count(serverAddress)) {
        emit read_ready(serverAddress, m_responses[serverAddress]);
    }
}

void MockModbusClient::on_write(int serverAddress, QModbusDataUnit mdu) {
    // Сохраняем записанное значение для последующей проверки
    m_writtenData[serverAddress] = mdu.values();
    // Эмулируем немедленный успех (можно добавить сигнал при необходимости)
}

void MockModbusClient::on_read(int serverAddress, QModbusDataUnit mdu) {
    sendSingleRequest(serverAddress, mdu);
}

void MockModbusClient::writeMultipleRegisters(int serverAddress, int startAddress,
                                              const QVector<quint16>& values) {
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, startAddress, values.size());
    unit.setValues(values);
    on_write(serverAddress, unit);
}

void MockModbusClient::setRegister(int serverAddress, const QString &reg_name, double value) {
    // Простейшая реализация: сохраняем в карте записанных значений
    QVector<quint16> data;
    data.append(static_cast<quint16>(value));
    m_writtenData[serverAddress] = data;
}

void MockModbusClient::setReadResponse(int serverAddress, const QModbusDataUnit &response) {
    m_responses[serverAddress] = response;
}

QVector<quint16> MockModbusClient::lastWrittenData(int serverAddress) const {
    auto it = m_writtenData.find(serverAddress);
    return it != m_writtenData.end() ? it->second : QVector<quint16>();
}
