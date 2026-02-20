#ifndef HTTPOPERATIONCONTROLLER_H
#define HTTPOPERATIONCONTROLLER_H

#include "QtWebApp/httpserver/httprequesthandler.h"

#include <QJsonObject>
#include <QJsonDocument>

#include "saks-lib-global.h"

class SAKS_LIB_EXPORT HttpOperationController : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
public:
    HttpOperationController(QObject* parent = nullptr);
    virtual ~HttpOperationController() {};
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);
signals:
    void info(QJsonObject &responseObject, int test_sequence_id = -1, int test_number = -1, int operation_id = -1);
    void run(QJsonObject &responseObject, int test_sequence_id = -1, int test_number = -1, int operation_id = -1);
};

#endif // HTTPOPERATIONCONTROLLER_H
