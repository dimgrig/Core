#include "HttpTestSequenceController.h"

#include "http_global.h"

extern stefanfrings::HttpSessionStore* session_store;

#include "Logger.h"
#include "http_functions.h"

HttpTestSequenceController::HttpTestSequenceController(QObject* parent)
    : HttpRequestHandler(parent) {

}

void HttpTestSequenceController::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) {
    QByteArray path = request.getPath();

    QString filename = get_string_value_from_json("path", request.getBody());
    int test_sequence_id = get_int_value_from_json("test_sequence_id", request.getBody());
    //qCDebug(logDebug) << "request.getBody()" << request.getBody() << "filename" << filename; //json

//    stefanfrings::HttpSession session = session_store->getSession(request, response, true);
//    session.set("filename", filename);

    if ((path == "/rv/info/test_sequence") || (path == "/rv/info/test_sequence/")) {
        QJsonObject responseObject;
        emit info(responseObject, test_sequence_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "info response" << responseObject << responseObject.keys();
    } else if ((path == "/rv/run/test_sequence") || (path == "/rv/run/test_sequence/")) {
        QJsonObject responseObject;
        emit run(responseObject, test_sequence_id);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "run response" << responseObject << responseObject.keys();
    } else if ((path == "/rv/file/test_sequence") || (path == "/rv/file/test_sequence/")) {
        QJsonObject responseObject;
        emit file(responseObject, filename);
        QJsonDocument responseDocument = QJsonDocument(responseObject);
        response.write(responseDocument.toJson());
        //qCDebug(logDebug()) << "run response" << responseObject << responseObject.keys();
    } else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.", true);
    }
}
