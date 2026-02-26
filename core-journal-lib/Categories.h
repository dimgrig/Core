#ifndef CATEGORIES_H
#define CATEGORIES_H

#include <QString>
#include <QStringLiteral>

#include "core-journal-lib-global.h"

namespace Category {
    CORE_JOURNAL_LIB_EXPORT inline const QString Authorization = QStringLiteral("Авторизация");
    CORE_JOURNAL_LIB_EXPORT inline const QString Calibration   = QStringLiteral("Калибровка");
    // CORE_JOURNAL_LIB_EXPORT inline const QString Utils         = QStringLiteral("Utils");
    // CORE_JOURNAL_LIB_EXPORT inline const QString Network       = QStringLiteral("Network");
}

#endif // CATEGORIES_H
