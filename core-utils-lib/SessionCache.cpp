#include "SessionCache.h"

SessionCache::SessionCache() {}

SessionCache& SessionCache::instance() {
    static SessionCache inst;
    return inst;
}
