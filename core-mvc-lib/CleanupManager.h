#ifndef CLEANUPMANAGER_H
#define CLEANUPMANAGER_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include <functional>

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT CleanupManager : public QObject
{
    Q_OBJECT
public:
    struct CleanupTask {
        QString tableName;
        QString timestampField;
        int keepDays;
        // Функция очистки, возвращает true при успехе, принимает callback прогресса и флаг отмены (указатель)
        std::function<int(const QString&, const QString&, int,
                           std::function<void(int,int)>, bool*)> cleanupFunc;
    };

    static CleanupManager* instance();

    void registerTask(const CleanupTask& task);
    void start(int intervalSec = 3600);
    void stop();

signals:
    void cleanupStarted(const QString& table);
    void cleanupProgress(const QString& table, int deleted, int total);
    void cleanupFinished(const QString& table, bool success);
    void cleanupError(const QString& table, const QString& error);

private slots:
    void onTimer();

private:
    explicit CleanupManager(QObject *parent = nullptr);
    ~CleanupManager();

    bool runCleanupTask(const CleanupTask& task, bool showProgress = true);

    QTimer m_timer;
    QList<CleanupTask> m_tasks;
    bool m_running = false;
    QMutex m_mutex; // защита m_running и m_tasks
};

#endif // CLEANUPMANAGER_H
