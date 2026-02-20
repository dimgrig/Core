#ifndef HTTP_FUNCTIONS_H
#define HTTP_FUNCTIONS_H

#include <QByteArray>
#include <QString>

int get_int_value_from_json(QString key, QByteArray json);
QString get_string_value_from_json(QString key, QByteArray json);

#endif // HTTP_FUNCTIONS_H
