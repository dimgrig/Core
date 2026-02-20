#ifndef HTTP_GLOBAL_H
#define HTTP_GLOBAL_H

#include "httpsession.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "templatecache.h"
//#include "filelogger.h"

#include "saks-lib-global.h"

extern stefanfrings::HttpSessionStore* session_store;

extern stefanfrings::StaticFileController* staticFileController;

extern stefanfrings::TemplateCache* template_cache;

//extern stefanfrings::Logger http_logger;

#endif // HTTP_GLOBAL_H
