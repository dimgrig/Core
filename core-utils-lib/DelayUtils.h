#ifndef DELAYUTILS_H
#define DELAYUTILS_H

#include <QObject>
#include <QTimer>
#include <QEventLoop>

#include "core-utils-lib-global.h"

class CORE_UTILS_LIB_EXPORT DelayUtils : public QObject {
    Q_OBJECT
public:
    // Неблокирующая задержка с обработкой событий
    static void delay(int milliseconds) {
        if (milliseconds <= 0) return;

        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);

        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(milliseconds);
        loop.exec();
    }

    // Задержка с обработкой событий и прогрессом
    static void delayWithProgress(int milliseconds, const std::function<void(int)>& progressCallback = nullptr) {
        if (milliseconds <= 0) return;

        const int step = 50; // шаг в мс
        int elapsed = 0;

        while (elapsed < milliseconds) {
            int currentStep = std::min(step, milliseconds - elapsed);

            QEventLoop loop;
            QTimer timer;
            timer.setSingleShot(true);

            QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            timer.start(currentStep);
            loop.exec();

            elapsed += currentStep;

            if (progressCallback) {
                progressCallback(elapsed * 100 / milliseconds);
            }
        }
    }

    // Асинхронная задержка (возвращает управление сразу)
    static QTimer* delayAsync(int milliseconds, std::function<void()> callback, QObject* parent = nullptr) {
        QTimer* timer = new QTimer(parent);
        timer->setSingleShot(true);

        QObject::connect(timer, &QTimer::timeout, [callback, timer]() {
            callback();
            timer->deleteLater();
        });

        timer->start(milliseconds);
        return timer;
    }
};

#endif // DELAYUTILS_H
