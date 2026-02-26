#ifndef ASTRA_LOGGER_H
#define ASTRA_LOGGER_H

#include <iostream>

#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QTextStream>

#include "CoreConfig.h"

Q_DECLARE_LOGGING_CATEGORY(logDebug)
Q_DECLARE_LOGGING_CATEGORY(logInfo)
Q_DECLARE_LOGGING_CATEGORY(logWarning)
Q_DECLARE_LOGGING_CATEGORY(logCritical)
Q_DECLARE_LOGGING_CATEGORY(logFatal)

// Тип для идентификатора категории (используется в журнале)
using CategoryId = quint16;

enum class LOG_LEV {
    DEBUG,
    INFO,
    WARNING,
    CRITICAL,
    FATAL,
    NONE_LOG
};

static QString logLevelToString(LOG_LEV level) {
    switch (level) {
    case LOG_LEV::DEBUG:    return QStringLiteral("Debug");
    case LOG_LEV::INFO:     return QStringLiteral("Info");
    case LOG_LEV::WARNING:  return QStringLiteral("Warning");
    case LOG_LEV::CRITICAL: return QStringLiteral("Critical");
    case LOG_LEV::FATAL:    return QStringLiteral("Fatal");
    default:                 return QStringLiteral("Unknown");
    }
}

static LOG_LEV logLevelFromString(const QString &level) {
    auto _level = level.toLower();
    if (_level.contains("debug")) {
        return LOG_LEV::DEBUG;
    } else if (_level.contains("info")) {
        return LOG_LEV::INFO;
    } else if (_level.contains("warning")) {
        return LOG_LEV::WARNING;
    } else if (_level.contains("critical")) {
        return LOG_LEV::CRITICAL;
    } else if (_level.contains("fatal")) {
        return LOG_LEV::FATAL;
    }
    return LOG_LEV::DEBUG;
}

class Logger {
public:
    static void log_init();
    static void log_delete();
    static void LogMessageOutput([[maybe_unused]] QtMsgType type, const QMessageLogContext &, const QString & str);
    static void LogFileOpening(int change);
    static void log_shutdown();

    static bool __log_enable;
    static bool __log_to_file;
    static bool __log_date;
    static std::string __log_file;
    static int __log_keep_days;
    static LOG_LEV __log_level;
};

//#define LOG_STDOUT

//#ifdef LOG_STDOUT
//    #define LOG(...) std::cout << __VA_ARGS__ << std::endl
//#elif LOG_PRINTF
//    #define LOG(...) printf(__VA_ARGS__)
//#elif defined LOG_QT
//    #define LOG(...)
//#else
//    #define LOG(...)
//#endif
#endif //ASTRA_LOGGER_H
