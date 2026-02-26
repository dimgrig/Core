#include "JournalBase.h"

#include <QLoggingCategory>

JournalBase::JournalBase() {}

JournalBase::~JournalBase() {}

void JournalBase::log(LOG_LEV level, CategoryId categoryId, const QString& msg) {
    // Получаем имя категории из кэша
    QString categoryName;
    {
        QMutexLocker locker(&m_cacheMutex);
        categoryName = m_reverseCache.value(categoryId, QString::number(categoryId));
    }
    QString fullMsg = QString("[%1] %2").arg(categoryName, msg);

    // Запись в файл
    logToFile(level, fullMsg);

    // Запись в БД
    insertLogEntry(QDateTime::currentDateTime(), level, categoryId, msg);
}

void JournalBase::logToFile(LOG_LEV level, const QString& fullMsg) {
    switch (level) {
    case LOG_LEV::DEBUG:
        qCDebug(logDebug()) << fullMsg;
        break;
    case LOG_LEV::INFO:
        qCInfo(logInfo()) << fullMsg;
        break;
    case LOG_LEV::WARNING:
        qCWarning(logWarning()) << fullMsg;
        break;
    case LOG_LEV::CRITICAL:
        qCCritical(logCritical()) << fullMsg;
        break;
    case LOG_LEV::FATAL:
        qCCritical(logFatal()) << fullMsg; // В Qt нет отдельного fatal, используем critical
        break;
    default:
        break;
    }
}

// Публичные методы логирования
void JournalBase::debug(CategoryId categoryId, const QString& msg) {
    log(LOG_LEV::DEBUG, categoryId, msg);
}
void JournalBase::info(CategoryId categoryId, const QString& msg) {
    log(LOG_LEV::INFO, categoryId, msg);
}
void JournalBase::warning(CategoryId categoryId, const QString& msg) {
    log(LOG_LEV::WARNING, categoryId, msg);
}
void JournalBase::critical(CategoryId categoryId, const QString& msg) {
    log(LOG_LEV::CRITICAL, categoryId, msg);
}
void JournalBase::fatal(CategoryId categoryId, const QString& msg) {
    log(LOG_LEV::FATAL, categoryId, msg);
}


void JournalBase::debug(const QString& categoryName, const QString& msg) {
    CategoryId cat = categoryId(categoryName);
    if (cat == 0) {
        // Категория не найдена в кэше – возможно, она ещё не зарегистрирована.
        // Регистрируем её (это безопасно, insertCategory вернёт существующий ID, если она уже есть в БД)
        cat = registerCategory(categoryName);
    }
    log(LOG_LEV::DEBUG, cat, msg);
}

void JournalBase::info(const QString& categoryName, const QString& msg) {
    CategoryId cat = categoryId(categoryName);
    if (cat == 0) {
        cat = registerCategory(categoryName);
    }
    log(LOG_LEV::INFO, cat, msg);
}

void JournalBase::warning(const QString& categoryName, const QString& msg) {
    CategoryId cat = categoryId(categoryName);
    if (cat == 0) {
        cat = registerCategory(categoryName);
    }
    log(LOG_LEV::WARNING, cat, msg);
}

void JournalBase::critical(const QString& categoryName, const QString& msg) {
    CategoryId cat = categoryId(categoryName);
    if (cat == 0) {
        cat = registerCategory(categoryName);
    }
    log(LOG_LEV::CRITICAL, cat, msg);
}

void JournalBase::fatal(const QString& categoryName, const QString& msg) {
    CategoryId cat = categoryId(categoryName);
    if (cat == 0) {
        cat = registerCategory(categoryName);
    }
    log(LOG_LEV::FATAL, cat, msg);
}


void JournalBase::debug(const QString& msg) {
    log(LOG_LEV::DEBUG, m_generalCat, msg);
}
void JournalBase::info(const QString& msg) {
    log(LOG_LEV::INFO, m_generalCat, msg);
}
void JournalBase::warning(const QString& msg) {
    log(LOG_LEV::WARNING, m_generalCat, msg);
}
void JournalBase::critical(const QString& msg) {
    log(LOG_LEV::CRITICAL, m_generalCat, msg);
}
void JournalBase::fatal(const QString& msg) {
    log(LOG_LEV::FATAL, m_generalCat, msg);
}

// Регистрация категории
CategoryId JournalBase::registerCategory(const QString& name) {
    QMutexLocker locker(&m_cacheMutex);
    auto it = m_categoryCache.find(name);
    if (it != m_categoryCache.end())
        return it.value();

    // Не найдено в кэше – обращаемся к БД (чисто виртуальный метод)
    CategoryId id = insertCategory(name);
    if (id != 0) {
        m_categoryCache[name] = id;
        m_reverseCache[id] = name;
    }
    return id;
}

CategoryId JournalBase::categoryId(const QString& name) const {
    QMutexLocker locker(&m_cacheMutex);
    return m_categoryCache.value(name, 0);
}

QString JournalBase::categoryName(CategoryId id) const {
    QMutexLocker locker(&m_cacheMutex);
    return m_reverseCache.value(id);
}
