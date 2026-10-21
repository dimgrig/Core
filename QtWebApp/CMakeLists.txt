core_add_library(QtWebApp
    QT      Core Network
    VERSION 1.9.0
    INCLUDES ${_qtw_subdirs}
)

target_sources(QtWebApp PRIVATE
    # Global
    QtWebAppGlobal.cpp
    QtWebAppGlobal.h

    # httpserver/ (из httpserver.pri)
    httpserver/httpglobal.cpp
    httpserver/httpglobal.h
    httpserver/httplistener.cpp
    httpserver/httplistener.h
    httpserver/httpconnectionhandler.cpp
    httpserver/httpconnectionhandler.h
    httpserver/httpconnectionhandlerpool.cpp
    httpserver/httpconnectionhandlerpool.h
    httpserver/httprequest.cpp
    httpserver/httprequest.h
    httpserver/httpresponse.cpp
    httpserver/httpresponse.h
    httpserver/httpcookie.cpp
    httpserver/httpcookie.h
    httpserver/httprequesthandler.cpp
    httpserver/httprequesthandler.h
    httpserver/httpsession.cpp
    httpserver/httpsession.h
    httpserver/httpsessionstore.cpp
    httpserver/httpsessionstore.h
    httpserver/staticfilecontroller.cpp
    httpserver/staticfilecontroller.h

    # templateengine/ (из templateengine.pri)
    templateengine/templateglobal.h
    templateengine/template.cpp
    templateengine/template.h
    templateengine/templateloader.cpp
    templateengine/templateloader.h
    templateengine/templatecache.cpp
    templateengine/templatecache.h

    # logging/ (из logging.pri)
    logging/dualfilelogger.cpp
    logging/dualfilelogger.h
    logging/filelogger.cpp
    logging/filelogger.h
    logging/logglobal.h
    logging/logger.cpp
    logging/logger.h
    logging/logmessage.cpp
    logging/logmessage.h
)

target_include_directories(QtWebApp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/httpserver>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/templateengine>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logging>
    $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}/QtWebApp/httpserver>
    $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}/QtWebApp/templateengine>
    $<INSTALL_INTERFACE:${CORE_INSTALL_INCLUDEDIR}/QtWebApp/logging>)
