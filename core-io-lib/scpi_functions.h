#ifndef SCPI_FUNCTIONS_H
#define SCPI_FUNCTIONS_H

#include <QByteArray>
#include <QVector>

void parse_float_array(QByteArray &ba, QVector<double> &res_v);

#endif // SCPI_FUNCTIONS_H
