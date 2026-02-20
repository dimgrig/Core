#ifndef SESSIONCACHE_H
#define SESSIONCACHE_H

#include <QString>
#include <QVariant>

#include "core-utils-lib-global.h"

#define session_cache SessionCache::instance()

class CORE_UTILS_LIB_EXPORT SessionCache {
public:
    static SessionCache& instance() {
        static SessionCache inst;
        return inst;
    }

    void set(const QString& key, const QVariant& value) {
        _cache[key] = value;
    }

    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) {
        return _cache.value(key, defaultValue);
    }

private:
    QMap<QString, QVariant> _cache;

private:
    SessionCache();
    SessionCache(const SessionCache&) = delete;
    SessionCache& operator=(const SessionCache&) = delete;
    ~SessionCache() {
        //std::cout << "SessionCache::~SessionCache()" << std::endl;
    }
};

#endif // SESSIONCACHE_H
