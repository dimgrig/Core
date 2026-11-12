#ifndef MIXINS_H
#define MIXINS_H

#include <QtDebug>
#include <QObject>

//#include <QDateTime>
//#include <QDateTimeEdit>

//class TimeRangesMixin {
//public:
//    TimeRangesMixin() = default;
//    ~TimeRangesMixin() = default;
//protected:
//    void set_last_day(QDateTimeEdit* edit);
//    void set_last_half_day(QDateTimeEdit* edit);
//    void set_last_hour(QDateTimeEdit* edit);
//};

template <typename C>
struct not_equal {
    friend bool operator!=(const C& lhs, const C& rhs) {
        return !(lhs == rhs);
    }
};

class NoCopyNoMoveBase {
public:
    NoCopyNoMoveBase() {};
    virtual ~NoCopyNoMoveBase() {};
    NoCopyNoMoveBase(const NoCopyNoMoveBase&) = delete; // конструктор копирования
    NoCopyNoMoveBase(NoCopyNoMoveBase&& other) = delete; // конструктор перемещения
    NoCopyNoMoveBase& operator=(const NoCopyNoMoveBase&) = delete; // оператор присваивания
    NoCopyNoMoveBase& operator=(NoCopyNoMoveBase&& other) = delete; // оператор перемещения
};

class CopyNoMoveBase {
public:
    CopyNoMoveBase() {};
    virtual ~CopyNoMoveBase() {};
    CopyNoMoveBase(const CopyNoMoveBase&) = default; // конструктор копирования
    CopyNoMoveBase(CopyNoMoveBase&& other) = delete; // конструктор перемещения
    CopyNoMoveBase& operator=(const CopyNoMoveBase&) = default; // оператор присваивания
    CopyNoMoveBase& operator=(CopyNoMoveBase&& other) = delete; // оператор перемещения
};

class NoCopyMoveBase {
public:
    NoCopyMoveBase() {};
    virtual ~NoCopyMoveBase() {};
    NoCopyMoveBase(const NoCopyMoveBase&) = delete; // конструктор копирования
    NoCopyMoveBase(NoCopyMoveBase&& other) = default; // конструктор перемещения
    NoCopyMoveBase& operator=(const NoCopyMoveBase&) = delete; // оператор присваивания
    NoCopyMoveBase& operator=(NoCopyMoveBase&& other) = default; // оператор перемещения
};

class CopyMoveBase {
public:
    CopyMoveBase() {};
    virtual ~CopyMoveBase() {};
    CopyMoveBase(const CopyMoveBase&) = default; // конструктор копирования
    CopyMoveBase(CopyMoveBase&& other) = default; // конструктор перемещения
    CopyMoveBase& operator=(const CopyMoveBase&) = default; // оператор присваивания
    CopyMoveBase& operator=(CopyMoveBase&& other) = default; // оператор перемещения
};

#endif // MIXINS_H
