#include "SerialClient.h"

#include "Logger.h"

SerialClient::SerialClient(QObject *parent) :
    QObject(parent)
{
    this->_serial->setReadBufferSize(64); //PACKET_MAX_LENGTH

    //qCDebug(logDebug()) << cfg.UART_PORT;
    connect(this->_serial, &QSerialPort::errorOccurred, this, &SerialClient::handleError);
    //connect(this->serial, &QSerialPort::readyRead, this, &SerialClient::readDataSlot);
    //connect(this->timer, &QTimer::timeout, this, &SerialClient::on_timer);
}

SerialClient::~SerialClient()
{
    this->closeSerialPort();
}

void SerialClient::set_async(bool async) {
    _async = async;
    if (async) {
        connect(this->_serial, &QSerialPort::readyRead, this, &SerialClient::readDataSlot);
    } else {
        disconnect(this->_serial, &QSerialPort::readyRead, this, &SerialClient::readDataSlot);
    }
}

void SerialClient::openSerialPort(QString port, int speed)
{
    if ((port.startsWith("tty")) || (port.startsWith("ptyp"))) {
        port = "/dev/" + port;
    }

    SerialSettings settings;
    //qCDebug(logDebug()) << cfg.UART_PORT << cfg.UART_SPEED;
    settings.name = port; //cfg.UART_PORT //"/dev/ttyS98"; //"COM3";
    settings.baudRate = speed; //cfg.UART_SPEED
    settings.dataBits = QSerialPort::Data8;
    settings.parity = QSerialPort::Parity::NoParity;
    settings.stopBits = QSerialPort::OneStop;
    settings.flowControl = QSerialPort::NoFlowControl;

    _serial->setPortName(settings.name);
    _serial->setBaudRate(settings.baudRate);
    _serial->setDataBits(settings.dataBits);
    _serial->setParity(settings.parity);
    _serial->setStopBits(settings.stopBits);
    _serial->setFlowControl(settings.flowControl);
    _serial->open(QIODevice::ReadWrite);
    if (this->isConnected()) {
        qCDebug(logInfo()) << "SERIAL: " << "connected" << port;
        emit port_opened();
        _serial->clear();
        //timer->start(this->TIMER_MS);
    } else {
        emit port_error("Serial: " + _serial->errorString());
        volatile QString str = _serial->errorString();
        qCDebug(logCritical()) << "SERIAL: " << _serial->errorString();
    }
}

void SerialClient::closeSerialPort()
{
    if (this->isConnected()) {
        _serial->clear();
        _serial->close();
        //timer->stop();
        qCDebug(logInfo()) << "SERIAL: " << "disconnected";
    }
    emit port_closed();
}

void SerialClient::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        qCDebug(logCritical()) << "SerialPortError" << _serial->errorString();
        emit port_error("Serial: " + _serial->errorString());
        closeSerialPort();
    }
}

void SerialClient::writeData(const char *buffer_tx, uint16_t length) {
    if (_serial->isOpen()) {
        _serial->write(buffer_tx, length);
#ifdef DEBUG_MSG_INTERFACES
            QByteArray txBuff = QByteArray::fromRawData(buffer_tx, length);
            qCDebug(logDebug()) << QT_HEX << "TCP: tx - "  << qPrintable(txBuff.toHex(' '));
#endif
    }
}

void SerialClient::writeData(const QByteArray &txBuff) {
    _serial->write(std::move(txBuff));
    _serial->waitForBytesWritten(1000);
    //serial->flush();
#ifdef DEBUG_MSG_INTERFACES
        qCDebug(logDebug()) << QT_HEX << "SERIAL: tx - "  << qPrintable(txBuff.toHex(' '));
#endif
}

std::string SerialClient::readData() {
    QByteArray rxBuff = _serial->readAll();

#ifdef DEBUG_MSG_INTERFACES
        qCDebug(logDebug()) << QT_HEX << "SERIAL: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif

    return rxBuff.toStdString();
}

void SerialClient::readDataSlot() {
    //serial->readyRead()
//    Sleeper sl;
//    sl.usleep(50000);

//    if (cfg.UART_DEBUG_MSG) {
//        qCDebug(logDebug()) << QT_HEX << "SERIAL: rx - " << serial->readBufferSize();
//    }

    QByteArray rxBuff = _serial->readAll();

#ifdef DEBUG_MSG_INTERFACES
        qCDebug(logDebug()) << QT_HEX << "SERIAL: rx - " << qPrintable(rxBuff.toHex(' ')) << "SIZE" << rxBuff.size();
#endif

    //rv
    emit read_data(rxBuff);

    //rmo
//    //fix only 32 byte rx error with CH340
//    if ((_rxBuff_overflow.size() == 0) && (rxBuff.size() > 1) && (rxBuff.at(1) > 32) && (rxBuff.size() < rxBuff.at(1))) { //rxBuff.size() == 32
////        if (cfg.DEBUG_MSG_UART) {
////            qCDebug(logDebug()) << "SERIAL: rx - BUFFER LESS - " << rxBuff.size() << rxBuff.at(1);
////        }
//        //rxBuff_overflow.clear();
//        _rxBuff_overflow = rxBuff;
//    } else {
//        if (_rxBuff_overflow.size() != 0) {
//            _rxBuff_overflow.append(rxBuff);
//            rxBuff.clear();
//            rxBuff = _rxBuff_overflow;
//            _rxBuff_overflow.clear();
//        }
////        if (cfg.DEBUG_MSG_UART) {
////            qCDebug(logDebug()) << "SERIAL: rx - BUFFER - " << rxBuff.size();
////        }
//        emit read_data(rxBuff);
//    }
}

void SerialClient::on_timer() {
    //TODO BASE check connection???
}
