#ifndef SCPI_FUNCTIONS_H
#define SCPI_FUNCTIONS_H

#include <QByteArray>
#include <QVector>

#include "core-io-lib-global.h"

void CORE_IO_LIB_EXPORT parse_float_array(QByteArray &ba, QVector<double> &res_v);

#endif // SCPI_FUNCTIONS_H
