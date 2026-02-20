#include "HttpOperationController.h"

#include "http_global.h"

extern stefanfrings::HttpSessionStore* session_store;

#include "Logger.h"
#include "http_functions.h"

HttpOperationController::HttpOperationController(QObject* parent)
    : HttpRequestHandler(parent) {

}

void HttpOperationController::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) {
    QByteArray path = request.getPath();

    int test_sequence_id = get_int_value_from_json("test_sequence_id", request.getBody());
    int test_number = get_int_value_from_json("test_id", request.getBody());
    int operation_id = get_int_value_from_json("operation_id", request.getBody());
    //qCDebug(logDebug) << "request.getBody()" << request.getBody() << "operation_id" << operation_id; //json

//    stefanfrings::HttpSession session = session_store->getSession(request, response, true);
//    session.set("operation_id", operation_id);

    if ((path == "/rv/info/operation") || (path == "/rv/info/operation/")) {
        QJsonObject responseObject;
        emit info(responseObject, test_sequence_id, test_number, operation_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "info response" << responseObject << responseObject.keys();
    } else if ((path == "/rv/run/operation") || (path == "/rv/run/operation/")) {
        QJsonObject responseObject;
        emit run(responseObject, test_sequence_id, test_number, operation_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "run response" << responseObject << responseObject.keys();
    } else if ((path == "/rv/run/next") || (path == "/rv/run/next/")) {
        QJsonObject responseObject;
        emit run(responseObject, test_sequence_id, test_number, operation_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "run response" << responseObject << responseObject.keys();
    } else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.", true);
    }
}
