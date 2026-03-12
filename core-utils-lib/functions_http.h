#ifndef FUNCTIONS_HTTP_H
#define FUNCTIONS_HTTP_H

#include <QByteArray>
#include <QString>

int get_int_value_from_json(QString key, QByteArray json);
QString get_string_value_from_json(QString key, QByteArray json);

#endif // FUNCTIONS_HTTP_H
