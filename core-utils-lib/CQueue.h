#ifndef CQUEUE_H
#define CQUEUE_H

#include <QCoreApplication>
#include <QQueue>
#include <QDebug>
#include <iostream>

#include "core-utils-lib-global.h"

#define CQUEUE_SIZE 10*60*60 //1 hour

//Кольцевой буфер на основе стандартной очереди
template <class T>
class CORE_UTILS_LIB_EXPORT CQueue : public QQueue <T> {
private:
    int count; //предельный размер
public:
    inline CQueue (int cnt) : QQueue<T>(),count(cnt) {
        //конструктор
    }
    inline void enqueue (const T &t) {
        //перегрузка метода постановки в очередь
        if (count==QQueue<T>::count()) {
            QQueue<T>::dequeue();
            //удалить старейший элемент по заполнении
        }
        QQueue<T>::enqueue(t);
    }
};
#endif // CQUEUE_H
