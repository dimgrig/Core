#ifndef HTTPBASECONTROLLER_H
#define HTTPBASECONTROLLER_H

#include "QtWebApp/httpserver/httprequesthandler.h"

#include "saks-lib-global.h"

class SAKS_LIB_EXPORT HttpBaseController : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
public:
    HttpBaseController(QObject* parent=0);
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);
};

#endif // HTTPBASECONTROLLER_H
