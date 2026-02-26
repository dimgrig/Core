#ifndef JOURNALACCESS_H
#define JOURNALACCESS_H

#include <QMutex>

#include "JournalBase.h"

#include "core-journal-lib-global.h"

class CORE_JOURNAL_LIB_EXPORT JournalAccess {
public:
    // Установить реализацию журнала (вызывается при инициализации приложения)
    static void setJournal(JournalBase* journal);

    // Получить указатель на текущий журнал (может быть nullptr)
    static JournalBase* journal();

    // Проверить, готов ли журнал
    static bool isReady();

private:
    static JournalBase* m_journal;
    static QMutex m_mutex;
};

#endif // JOURNALACCESS_H
