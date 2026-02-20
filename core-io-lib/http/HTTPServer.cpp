#include "HTTPServer.h"
//#include "ui_http_server.h"

#include <QSettings>

#include "Config.h"

#include "HttpRequestMapper.h"

#include "http_global.h"

extern stefanfrings::HttpSessionStore* session_store;
//extern stefanfrings::FileLogger* logger;

HTTPServer::HTTPServer(QWidget *parent)
    : QWidget(parent)
    //, ui(new Ui::fhttp_server)
{
    //ui->setupUi(this);

    //settings file may be the same? but pick up several times with different sections in begiGroup

    // Configure logging - //duplicates Logger
//    QSettings* log_settings = new QSettings("webapp.ini", QSettings::IniFormat, qApp);
//    log_settings->beginGroup("logging");
//    logger = new stefanfrings::FileLogger(log_settings, 10000, qApp);
//    logger->installMsgHandler();

    // Session store
    QSettings* session_settings = new QSettings("webapp.ini", QSettings::IniFormat, qApp);
    session_settings->beginGroup("sessions");
    session_store = new stefanfrings::HttpSessionStore(session_settings, qApp);

    // HTTP server
    // Settings can also be loaded from a file
    QSettings* listener_settings = new QSettings("webapp.ini", QSettings::IniFormat, qApp);
    listener_settings->beginGroup("listener"); //must be - important - crashes otherwise
    //qCDebug(logDebug()) << listener_settings->value("port", 0) << listener_settings->value("maxThreads", 0);

    // Start the webserver
    _request_mapper = new HttpRequestMapper(qApp);
    _http_listener = new stefanfrings::HttpListener(listener_settings, _request_mapper, qApp);
}

HTTPServer::~HTTPServer() {
    if (_http_listener) {
        _http_listener->close();
    }
}

//void HTTPServer::process(const TestSequence *tests) {
//    this->_request_mapper->process(tests);
//}

