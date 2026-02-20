#include "HttpTestController.h"

#include "http_global.h"

extern stefanfrings::HttpSessionStore* session_store;

#include "Logger.h"
#include "http_functions.h"

HttpTestController::HttpTestController(QObject* parent)
    : HttpRequestHandler(parent) {

}

void HttpTestController::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) {
    QByteArray path = request.getPath();

    int test_sequence_id = get_int_value_from_json("test_sequence_id", request.getBody());
    int test_id = get_int_value_from_json("test_id", request.getBody());
    //qCDebug(logDebug) << "request.getBody()" << request.getBody() << "test_id" << test_id; //json

//    stefanfrings::HttpSession session = session_store->getSession(request, response, true);
//    session.set("test_id", test_id);

    if ((path == "/rv/info/test") || (path == "/rv/info/test/")) {
        QJsonObject responseObject;
        emit info(responseObject, test_sequence_id, test_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "info response" << responseObject << responseObject.keys();
    } else if ((path == "/rv/run/test") || (path == "/rv/run/test/")) {
        QJsonObject responseObject;
        emit run(responseObject, test_sequence_id, test_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "run response" << responseObject << responseObject.keys();
    } else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.", true);
    }
}

//if (!responseObject.isEmpty()) {
//    QJsonDocument responseDocument = QJsonDocument(responseObject);
//    response.write(responseDocument.toJson());
//    session_store->removeSession(session);
//}
