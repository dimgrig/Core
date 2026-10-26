#ifndef TESTJOURNAL_H
#define TESTJOURNAL_H

#include <QObject>
#include <QDateTime>
#include <QVector>

#include <core-journal-lib/JournalBase.h>
#include <core-journal-lib/JournalAccess.h>

// Тестовая реализация JournalBase
class TestJournal : public JournalBase {
public:
    TestJournal();
    ~TestJournal() override;

    // Переопределения виртуальных методов
    CategoryId insertCategory(const QString& name) override;
    void insertLogEntry(const QDateTime& timestamp, LOG_LEV level,
                        CategoryId categoryId, const QString& msg) override;
    void loadCategoriesFromModel() override;

    // Структура для записи лога
    struct LogEntry {
        QDateTime timestamp;
        LOG_LEV level;
        CategoryId category;
        QString message;
    };

    // Счетчики и данные для проверки
    int insertCategoryCallCount = 0;
    QString lastInsertCategoryName;
    QVector<LogEntry> loggedEntries;

    // Управление возвращаемыми ID
    CategoryId nextCategoryId = 1;
    QHash<QString, CategoryId> registeredCategories; // симулируем БД

    void reset();
};

// Тестовый класс для JournalAccess
class JournalAccessTest : public QObject {
    Q_OBJECT
private slots:
    void init();
    void testInitialState();
    void testSetAndGet();
    void testSetNull();
};

// Тестовый класс для JournalBase
class JournalBaseTest : public QObject {
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testGeneralCategory();
    void testRegisterCategory();
    void testCategoryIdAndName();
    void testLoggingWithCategoryId();
    void testLoggingWithoutCategory();
    void testLoggingWithCategoryName();
    void testCacheConsistency();

private:
    TestJournal* m_testJournal = nullptr;
};

#endif // TESTJOURNAL_H
