#ifndef ASTRA_SLEEPER_H
#define ASTRA_SLEEPER_H

#include <QThread>
#include <QEventLoop>
#include <QTimer>

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){
        QThread::usleep(usecs);
    }
    static void msleep(unsigned long msecs){
        //QThread::msleep(msecs);
        QEventLoop loop;
        QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
        loop.exec();
    }
    static void sleep(unsigned long secs){
        //QThread::sleep(secs);
        QEventLoop loop;
        QTimer::singleShot(secs*1000, &loop, &QEventLoop::quit);
        loop.exec();
    }
};

#endif //ASTRA_SLEEPER_H
