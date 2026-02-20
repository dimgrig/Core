#ifndef HTTPREQUESTMAPPER_H
#define HTTPREQUESTMAPPER_H

#include "QtWebApp/httpserver/httprequesthandler.h"

#include "HttpBaseController.h"
#include "HttpTestSequenceController.h"
#include "HttpTestController.h"
#include "HttpOperationController.h"

namespace stefanfrings {
class HttpSession;
}

class SAKS_LIB_EXPORT HttpRequestMapper : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
public:
    HttpRequestMapper(QObject* parent=0);
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

    HttpTestSequenceController* get_httpTestSequenceController() const {return _httpTestSequenceController;};
    HttpTestController* get_httpTestController() const {return _httpTestController;};
    HttpOperationController* get_httpOperationController() const {return _httpOperationController;};

    //void process(const TestSequence *tests);
private:
    //void clear_session(stefanfrings::HttpSession &session);

    HttpBaseController* _httpBaseController = new HttpBaseController(this);
    HttpTestSequenceController* _httpTestSequenceController = new HttpTestSequenceController(this);
    HttpTestController* _httpTestController = new HttpTestController(this);
    HttpOperationController* _httpOperationController = new HttpOperationController(this);

    //std::vector<QByteArray> _session_id_v;
};

#endif // HTTPREQUESTMAPPER_H
