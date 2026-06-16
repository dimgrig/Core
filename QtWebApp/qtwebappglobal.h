#ifndef QTWEBAPPGLOBAL_H
#define QTWEBAPPGLOBAL_H

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
