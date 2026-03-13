#ifndef FUNCTIONS_HTTP_H
#define FUNCTIONS_HTTP_H

#include <QByteArray>
#include <QString>

#include "core-utils-lib-global.h"

CORE_UTILS_LIB_EXPORT int get_int_value_from_json(QString key, QByteArray json);
CORE_UTILS_LIB_EXPORT QString get_string_value_from_json(QString key, QByteArray json);

#endif // FUNCTIONS_HTTP_H
