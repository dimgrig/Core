#include "CleanupManager.h"

#include <QProgressDialog>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThread>

#include "JournalAccess.h"
#include "Categories.h"

CleanupManager* CleanupManager::instance()
{
    static CleanupManager mgr;
    return &mgr;
}

CleanupManager::CleanupManager(QObject *parent)
    : QObject(parent)
{
}

CleanupManager::~CleanupManager()
{
    stop();
}

void CleanupManager::registerTask(const CleanupTask& task)
{
    QMutexLocker locker(&m_mutex);
    m_tasks.append(task);
}

void CleanupManager::start(int intervalSec)
{
    QMutexLocker locker(&m_mutex);
    if (m_timer.isActive())
        return;

    m_timer.setInterval(intervalSec * 1000);
    connect(&m_timer, &QTimer::timeout, this, &CleanupManager::onTimer);
    m_timer.start();

    // Запускаем немедленно
    QMetaObject::invokeMethod(this, "onTimer", Qt::QueuedConnection);
}

void CleanupManager::stop()
{
    QMutexLocker locker(&m_mutex);
    m_timer.stop();
    // можно прервать текущую операцию, если требуется – добавим флаг отмены
}

void CleanupManager::onTimer()
{
    QMutexLocker locker(&m_mutex);
    if (m_running || m_tasks.isEmpty())
        return;

    m_running = true;
    QList<CleanupTask> tasksCopy = m_tasks;
    locker.unlock();

    for (const CleanupTask& task : tasksCopy) {
        // Выполняем задачу с показом прогресса
        bool success = runCleanupTask(task, true);
        if (!success) {
            // Можно логировать ошибку
        }
        // Если нужно прервать по какому-то условию – можно добавить
    }

    locker.relock();
    m_running = false;
}

bool CleanupManager::runCleanupTask(const CleanupTask& task, bool showProgress)
{
    emit cleanupStarted(task.tableName);

    // Подготавливаем прогресс-диалог (если требуется)
    QScopedPointer<QProgressDialog> progressDialog;
    if (showProgress) {
        progressDialog.reset(new QProgressDialog(
            QString("Очистка таблицы %1...").arg(task.tableName),
            "Отмена", 0, 0, nullptr));
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setMinimumDuration(2000); // показывать, если >2 сек
        progressDialog->setAutoClose(true);
        progressDialog->setAutoReset(false);
        progressDialog->setRange(0, 0); // сначала неопределённый
        progressDialog->show();
    }

    bool cancel = false;
    // Соединяем кнопку отмены
    if (progressDialog) {
        QObject::connect(progressDialog.data(), &QProgressDialog::canceled, [&]() {
            cancel = true;
        });
    }

    // Вызываем функцию очистки (она выполняется в том же потоке)
    // Передаём callback для прогресса
    int deletedCount = task.cleanupFunc(
        task.tableName,
        task.timestampField,
        task.keepDays,
        [&](int deleted, int total) {
            // Обновляем прогресс-диалог
            if (progressDialog) {
                if (total > 0 && progressDialog->maximum() != total) {
                    progressDialog->setRange(0, total);
                }
                progressDialog->setValue(deleted);
                progressDialog->setLabelText(
                    QString("Удалено %1 из %2 записей из таблицы %3")
                        .arg(deleted).arg(total).arg(task.tableName)
                    );
                // Обрабатываем события, чтобы диалог обновлялся и кнопка отмены работала
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        },
        &cancel
        );

    if (progressDialog) {
        progressDialog->close();
    }

    // Логирование
    bool success = false;
    auto* j = JournalAccess::journal();
    if (deletedCount >= 0) {
        success = true;
        if (j) {
            j->info(Category::Database,
                    QString("Очистка таблицы %1: удалено %2 записей (старше %3 дней)")
                        .arg(task.tableName).arg(deletedCount).arg(task.keepDays));
        }
        emit cleanupFinished(task.tableName, success);
    } else {
        success = false;
        if (j) {
            j->warning(Category::Database,
                       QString("Ошибка при очистке таблицы %1 (код ошибки %2)")
                           .arg(task.tableName).arg(deletedCount));
        }
        emit cleanupError(task.tableName, "Ошибка при удалении записей");
        emit cleanupFinished(task.tableName, success);
    }

    return success;
}
