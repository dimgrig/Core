#ifndef FUNCTIONS_DEGREES_H
#define FUNCTIONS_DEGREES_H

#include <vector>

#include <QString>

#include "core-utils-lib-global.h"

void CORE_UTILS_LIB_EXPORT FunctionsDegreesTest();

// For weird Microsoft compilers, e.g. MSVC9
#ifndef M_PI
#define M_PI 3.14159265358
#endif

qreal CORE_UTILS_LIB_EXPORT radiansFromDegrees(qreal degrees);
qreal CORE_UTILS_LIB_EXPORT degreesFromRadians(qreal radians);
double CORE_UTILS_LIB_EXPORT cnt_to_radians(double cnt, int interpolation);
double CORE_UTILS_LIB_EXPORT radians_to_cnt(double radians, int interpolation);
QString CORE_UTILS_LIB_EXPORT dms_to_string(int degrees, int min, double sec);
void CORE_UTILS_LIB_EXPORT dms_from_string(const QString dms, double &degrees, double &minutes, double &seconds);
QString CORE_UTILS_LIB_EXPORT radians_to_dms(double radians, int precision = 5);
QString CORE_UTILS_LIB_EXPORT degrees_to_dms(double degrees, int precision = 5);
double CORE_UTILS_LIB_EXPORT dms_to_radians(QString dms);
double CORE_UTILS_LIB_EXPORT dms_to_degrees(QString dms);
double CORE_UTILS_LIB_EXPORT dms_to_radians(double degrees, double minutes, double seconds);
double CORE_UTILS_LIB_EXPORT dms_to_degrees(double degrees, double minutes, double seconds);

#endif // FUNCTIONS_DEGREES_H
