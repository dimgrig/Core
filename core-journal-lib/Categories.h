#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <QString>
#include <QStringLiteral>

#include "core-journal-lib-global.h"

namespace Category {
    inline const QString General       = QStringLiteral("Общее");
    inline const QString Authorization = QStringLiteral("Авторизация");
    inline const QString Calibration   = QStringLiteral("Калибровка");
    // inline const QString Utils         = QStringLiteral("Utils");
    // inline const QString Network       = QStringLiteral("Network");
}

#endif // CATEGORIES_H
