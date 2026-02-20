#ifndef HTTPTESTCONTROLLER_H
#define HTTPTESTCONTROLLER_H

#include "QtWebApp/httpserver/httprequesthandler.h"

#include <QJsonObject>
#include <QJsonDocument>

#include "saks-lib-global.h"

class SAKS_LIB_EXPORT HttpTestController : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
public:
    HttpTestController(QObject* parent = nullptr);
    virtual ~HttpTestController() {};
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);
signals:
    void info(QJsonObject &responseObject, int test_sequence_id = -1, int test_id = -1);
    void run(QJsonObject &responseObject, int test_sequence_id = -1, int test_id = -1);
};

#endif // HTTPTESTCONTROLLER_H
