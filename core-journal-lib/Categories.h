#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <QString>
#include <QStringLiteral>

#include "core-journal-lib-global.h"

namespace Category {
    inline const QString General       = QStringLiteral("Общее");
    inline const QString System        = QStringLiteral("Системные");
    inline const QString Authorization = QStringLiteral("Авторизация");
    inline const QString Users         = QStringLiteral("Пользователи");
    inline const QString Settings      = QStringLiteral("Настройки");
    inline const QString Modbus        = QStringLiteral("Modbus");
    inline const QString Devices       = QStringLiteral("Устройства");
    inline const QString Calibration   = QStringLiteral("Калибровка");
    inline const QString Database      = QStringLiteral("База данных");
    inline const QString IO            = QStringLiteral("IO");
    // inline const QString Utils         = QStringLiteral("Utils");
    // inline const QString Network       = QStringLiteral("Network");
}

#endif // CATEGORIES_H
