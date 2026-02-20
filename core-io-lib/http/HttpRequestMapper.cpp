#include "HttpRequestMapper.h"

#include "http_global.h"

extern stefanfrings::HttpSessionStore* session_store;

#include "Logger.h"

HttpRequestMapper::HttpRequestMapper(QObject* parent)
    : HttpRequestHandler(parent) {
    // empty
}

void HttpRequestMapper::service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response) {
    QByteArray path = request.getPath();
    //qCDebug(logDebug()) << QString("RequestMapper: path=%1").arg(path.data());

    //stefanfrings::HttpSession session = session_store->getSession(request, response, true);
    //this->_session_id_v.push_back(session.getId());

    //qCDebug(logDebug) << "request.getParameterMap()" << request.getParameterMap(); //parameter
    //qCDebug(logDebug) << "request.getBody()" << request.getBody(); //json

    //QByteArray test_id = request.getParameter("test_id");
    //QByteArray operation_id = request.getParameter("operation_id");
//        if (test_id.size() > 0) {
//            session.set("test_id", test_id);
//        }
//        if (operation_id.size() > 0) {
//            session.set("operation_id", operation_id);
//        }

    if (path == "/" || path == "/help") {
        _httpBaseController->service(request, response);
    } else if ((path == "/rv/info/test_sequence") || (path == "/rv/info/test_sequence/") ||
               (path == "/rv/run/test_sequence") || (path == "/rv/run/test_sequence/") ||
               (path == "/rv/file/test_sequence") || (path == "/rv/file/test_sequence/")) {
        _httpTestSequenceController->service(request, response);
    } else if ((path == "/rv/info/test") || (path == "/rv/run/test") ||
               (path == "/rv/info/test/") || (path == "/rv/run/test/")) {
        _httpTestController->service(request, response);
    } else if ((path == "/rv/info/operation") || (path == "/rv/info/operation/") ||
               (path == "/rv/run/operation") || (path == "/rv/run/operation/") ||
               (path == "/rv/run/next") || (path == "/rv/run/next/")) {
        _httpOperationController->service(request, response);
    } else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.", true);
    }

    //this->clear_session(session);
}

//void HttpRequestMapper::process(const TestSequence *tests) {
//    Q_UNUSED(tests);

//    if (!_session_id_v.empty()) {
//        qCDebug(logDebug()) << "HttpRequestMapper::process" << _session_id_v.size();
//        for (const auto &session_id : _session_id_v) {
//            //auto session_id = _session_id_v.front();
//            if (session_id.size() > 0) {
//                stefanfrings::HttpSession session = session_store->getSession(session_id);

//                int test_number = session.get("test_id").toInt();
//                int operation_id = session.get("operation_id").toInt();
//                qCDebug(logDebug()) << "HttpRequestMapper::process" << test_number << operation_id;

//                if (test_number != 0) {
//                    if (test_number == -1) {
//                        if (tests->get_test_number_current() != -1) {
//                            //run next test
//                            //tests->info_operation(responseObject, operation_id);
//                        } else {
//                            //no next test
//                            //sync answered
//                        }
//                    } else {
//                        //run selected test
//                        //TODO
//                    }
//                }



//                session_store->removeSession(session);
//            }
//        }
//    }
//}

//void HttpRequestMapper::clear_session(stefanfrings::HttpSession &session) {
//    if (session_store->getSession(session.getId()).isNull()) {
//        _session_id_v.erase(std::remove(_session_id_v.begin(), _session_id_v.end(), session.getId()), _session_id_v.end());
//    }
//}
