#ifndef HTTPTESTSEQUENCECONTROLLER_H
#define HTTPTESTSEQUENCECONTROLLER_H

#include "QtWebApp/httpserver/httprequesthandler.h"

#include <QJsonObject>
#include <QJsonDocument>

#include "saks-lib-global.h"

class SAKS_LIB_EXPORT HttpTestSequenceController : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
public:
    HttpTestSequenceController(QObject* parent = nullptr);
    virtual ~HttpTestSequenceController() {};
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);
signals:
    void info(QJsonObject &responseObject, int test_sequence_id = -1);
    void run(QJsonObject &responseObject, int test_sequence_id = -1);
    void file(QJsonObject &responseObject, const QString &filename);
};

#endif // HTTPTESTSEQUENCECONTROLLER_H
