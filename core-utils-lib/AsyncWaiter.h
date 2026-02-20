#ifndef ASYNCWAITER_H
#define ASYNCWAITER_H

#include <QProgressBar>
#include <QTimer>
#include <QElapsedTimer>
#include <QPushButton>

#include "core-utils-lib-global.h"

class CORE_UTILS_LIB_EXPORT AsyncWaiter : public QObject {
    Q_OBJECT
public:
    AsyncWaiter(QProgressBar* progress_bar, QObject* parent = nullptr);

    void start_waiting_s(int total_time_s);
    void start_waiting_ms(int total_time_ms);
    void cancel();

signals:
    void finished();

private slots:
    void update_progress();

private:
    QProgressBar* _progress_bar;
    QTimer* _timer;
    QElapsedTimer _elapsed_timer;
    int _total_time = 0;
};

#endif // ASYNCWAITER_H
