#ifndef CLIENTHELPERS_H
#define CLIENTHELPERS_H

#include <type_traits>

#include <QEventLoop>
#include <QIODevice>
#include <QTimer>

#include "Logger.h"

// Ожидание любого сигнала от объекта sender с таймаутом в мс.
// Возвращает true, если сигнал пришёл до истечения таймаута.
template <typename Sender, typename Signal>
bool waitForSignal(Sender* sender, Signal signal, int timeoutMs) {
    static_assert(std::is_member_function_pointer<Signal>::value,
                  "Signal must be a member function pointer (e.g., &Class::signal)");

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(sender, signal, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();
    return timer.isActive();
}

// Ожидание сигнала после выполнения действия (поддерживает как синхронные, так и асинхронные сигналы)
template <typename Sender, typename Signal, typename Callable>
bool waitForSignalAfterCall(Sender* sender, Signal signal, int timeoutMs, Callable action) {
    bool signalReceived = false;
    // Временное соединение для перехвата синхронного сигнала
    QMetaObject::Connection conn = QObject::connect(sender, signal, [&](){
        signalReceived = true;
    });
    action(); // выполняем действие – сигнал может прийти синхронно
    QObject::disconnect(conn);

    if (signalReceived) {
        return true; // синхронный сигнал уже пришёл
    }

    // Асинхронное ожидание
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(sender, signal, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();
    return timer.isActive();
}

template <typename Device>
QByteArray readDataWithTimeout(Device* device, int timeoutMs) {
    if (!device || !device->isOpen()) return {};

    QByteArray result;
    // Сначала вычитываем всё, что уже есть
    while (device->bytesAvailable())
        result += device->readAll();

    if (result.isEmpty()) {
        // Данных нет – ждём readyRead
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(device, &Device::readyRead, &loop, [&]() {
            while (device->bytesAvailable())
                result += device->readAll();
            // Не выходим из цикла, продолжаем накапливать до таймаута
        });
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(timeoutMs);
        loop.exec();
        QObject::disconnect(device, &Device::readyRead, &loop, nullptr);
        QObject::disconnect(&timer, &QTimer::timeout, &loop, nullptr);
    } else {
        // Данные уже были – нужно решить, ждать ли ещё?
        // Можно запустить короткий таймер (например, 100 мс) на досборку
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(device, &Device::readyRead, &loop, [&]() {
            while (device->bytesAvailable())
                result += device->readAll();
            // Не выходим, продолжаем накопление до таймаута
        });
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        // Короткий таймаут на досборку оставшихся фрагментов
        timer.start(100); // 100 мс – достаточно для прихода следующих пакетов
        loop.exec();
        QObject::disconnect(device, &Device::readyRead, &loop, nullptr);
        QObject::disconnect(&timer, &QTimer::timeout, &loop, nullptr);
    }

    return result;
}
#endif // CLIENTHELPERS_H
