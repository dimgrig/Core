#ifndef QTWEBAPPGLOBAL_H
#define QTWEBAPPGLOBAL_H

// Q_DECL_EXPORT / Q_DECL_IMPORT живут здесь
#include <QtCore/qglobal.h>

// Макрос ОБЯЗАН быть определён до любого #include ниже —
// иначе httpsession.h (и другие) увидят неизвестный идентификатор.
#if defined(QTWEBAPP_LIBRARY)
#  define QTWEBAPP_EXPORT Q_DECL_EXPORT
#else
#  define QTWEBAPP_EXPORT Q_DECL_IMPORT
#endif

#include "httpsession.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "templatecache.h"
//#include "filelogger.h"

extern stefanfrings::HttpSessionStore* session_store;

extern stefanfrings::StaticFileController* staticFileController;

extern stefanfrings::TemplateCache* template_cache;

//extern stefanfrings::Logger http_logger;

#endif // QTWEBAPPGLOBAL_H
