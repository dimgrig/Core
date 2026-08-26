#include "functions_http.h"

#include <QJsonObject>
#include <QJsonDocument>

#include "Logger.h"

double get_double_value_from_json(QString key, QByteArray json) {
    double res = -1;

    // get the root object
    QJsonDocument itemDoc = QJsonDocument::fromJson(json);
    QJsonObject requestObject = itemDoc.object();

    if (requestObject.contains(key)) {
        res = requestObject.value(key).toDouble();
    }

    return res;
}

int get_int_value_from_json(QString key, QByteArray json) {
    int res = -1;

    // get the root object
    QJsonDocument itemDoc = QJsonDocument::fromJson(json);
    QJsonObject requestObject = itemDoc.object();

    if (requestObject.contains(key)) {
        res = requestObject.value(key).toInt();
    }

    return res;
}

QString get_string_value_from_json(QString key, QByteArray json) {
    QString res = "";

    // get the root object
    QJsonDocument itemDoc = QJsonDocument::fromJson(json);
    QJsonObject requestObject = itemDoc.object();

    if (requestObject.contains(key)) {
        res = requestObject.value(key).toString();
    }

    return res;
}

//    // get the response object
//    QJsonValue response = rootObject.value("response");
//    QJsonObject responseObj = response.toObject();

    // print out the list of keys ("count")
//    QStringList keys = requestObject.keys();
//    qCDebug(logDebug()) << "requestObject.keys()" << keys;

//    // print the value of the key "count")
//    qDebug() << responseObj.value("count");

//    // get the array of items
//    QJsonValue itemArrayValue = responseObj.value("items");

//    // check we have an array
//    if(itemArrayValue.isArray())
//    {
//        // get the array as a JsonArray
//        QJsonArray itemArray = itemArrayValue.toArray();
//    }
