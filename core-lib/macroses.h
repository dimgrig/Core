#ifndef MACROSES_H
#define MACROSES_H

#include "Logger.h"

void MacrosesTest();

#define AS_KV(x) (string_clearify((#x))).c_str() << " = " << x << "\n"

#define ARRAY_LEN(x)            (sizeof(x) / sizeof((x)[0]))

#define issigned(t) (((t)(-1)) < ((t) 0))

#define umaxof(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define smaxof(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))
//max value of type
#define maxof(t) ((unsigned long long) (issigned(t) ? smaxof(t) : umaxof(t)))

#define UNUSED(x) (void)(x)

#define CLAMP(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)
//ANON_VAR - генерирует уникальное имя для переменной х
#ifdef __COUNTER__
    #define ANON_VAR(x) CONCAT(x, __COUNTER__)
#else
    #define ANON_VAR(x) CONCAT(x, __LINE__)
#endif

#define SET_TRISTATE(S) \
S->exclusive(); \
S->addButton(ui->S##_Down, 2); \
S->addButton(ui->S##_Mid, 1); \
S->addButton(ui->S##_Up, 0);

#define IMITATOR_FRAME_HIDING(btn, frame) \
if (btn->isChecked()) { \
    frame->hide(); \
} else { \
    frame->show(); \
}

#define CHOICE_TRISTATE(S, S1, S2) \
if (!S->isChecked()) { \
    ui->S1##_Down->setProperty("grayed", true); \
    ui->S1##_Mid->setProperty("grayed", true); \
    ui->S1##_Up->setProperty("grayed", true); \
    ui->S2##_Down->setProperty("grayed", false); \
    ui->S2##_Mid->setProperty("grayed", false); \
    ui->S2##_Up->setProperty("grayed", false); \
} else { \
    ui->S1##_Down->setProperty("grayed", false); \
    ui->S1##_Mid->setProperty("grayed", false); \
    ui->S1##_Up->setProperty("grayed", false); \
    ui->S2##_Down->setProperty("grayed", true); \
    ui->S2##_Mid->setProperty("grayed", true); \
    ui->S2##_Up->setProperty("grayed", true); \
} \
ui->S1##_Down->setStyleSheet(ui->S1##_Down->styleSheet()); \
ui->S1##_Mid->setStyleSheet(ui->S1##_Mid->styleSheet()); \
ui->S1##_Up->setStyleSheet(ui->S1##_Up->styleSheet()); \
ui->S2##_Down->setStyleSheet(ui->S2##_Down->styleSheet()); \
ui->S2##_Mid->setStyleSheet(ui->S2##_Mid->styleSheet()); \
ui->S2##_Up->setStyleSheet(ui->S2##_Up->styleSheet());

#define CHECK_SIZE(text, x, y)                                                      \
    if (x.size() != y.size()) {                                                         \
        qCDebug(logFatal()) << text << "boost::combine sizes not equal: "          \
                            << x.size() << "!=" << y.size()                             \
                            << "(" << #x << "!=" << #y << ")";                          \
    }                                                                                   \

#endif //MACROSES_H
