#include "HttpBaseController.h"

#include "http_global.h"

extern stefanfrings::HttpSessionStore* session_store;

HttpBaseController::HttpBaseController(QObject* parent)
    : HttpRequestHandler(parent) {

}

void HttpBaseController::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) {
    QByteArray path = request.getPath();

    //stefanfrings::HttpSession session = session_store->getSession(request, response, true);

    if (path == "/") {
        response.write("ATML ACS", true);
    } else if (path == "/help") {
        response.write("ATML ACS usage: section in development", true);
    } else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.", true);
    }
}
