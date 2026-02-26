#include "JournalAccess.h"

JournalBase* JournalAccess::m_journal = nullptr;
QMutex JournalAccess::m_mutex;

void JournalAccess::setJournal(JournalBase* journal) {
    QMutexLocker locker(&m_mutex);
    m_journal = journal;
}

JournalBase* JournalAccess::journal() {
    QMutexLocker locker(&m_mutex);
    return m_journal;
}

bool JournalAccess::isReady() {
    QMutexLocker locker(&m_mutex);
    return m_journal != nullptr;
}
