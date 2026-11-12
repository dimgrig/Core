#include "AsyncWaiter.h"

AsyncWaiter::AsyncWaiter(QProgressBar* progress_bar, QObject* parent)
    : QObject(parent),
    _progress_bar(progress_bar),
    _timer(new QTimer(this))
{
    connect(_timer, &QTimer::timeout, this, &AsyncWaiter::update_progress);
}

void AsyncWaiter::start_waiting_s(int total_time_s) {
    this->start_waiting_ms(total_time_s * 1000);
}

void AsyncWaiter::start_waiting_ms(int total_time_ms) {
    _total_time = total_time_ms;
    _elapsed_timer.start();
    _progress_bar->setValue(0);
    _timer->start(50); // Обновление каждые 50 мс
}

void AsyncWaiter::cancel() {
    _timer->stop();
    _progress_bar->reset();
}

void AsyncWaiter::update_progress() {
    int elapsed = _elapsed_timer.elapsed();
    int progress = qMin(100, static_cast<int>((elapsed * 100.0) / _total_time));

    _progress_bar->setValue(progress);

    if (elapsed >= _total_time) {
        _timer->stop();
        emit finished();
    }
}
