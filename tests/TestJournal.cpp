#include "TestJournal.h"

#include <QTest>
#include <QThread>

// ==================== TestJournal implementation ====================
TestJournal::TestJournal() : JournalBase() {
    // Устанавливаем generalCategory, как это сделал бы наследник
    m_generalCat = registerCategory("General");
}

TestJournal::~TestJournal() {}

CategoryId TestJournal::insertCategory(const QString& name) {
    insertCategoryCallCount++;
    lastInsertCategoryName = name;
    // Симулируем вставку в БД: если уже есть, возвращаем существующий ID
    if (registeredCategories.contains(name)) {
        return registeredCategories[name];
    }
    CategoryId id = nextCategoryId++;
    registeredCategories[name] = id;
    return id;
}

void TestJournal::insertLogEntry(const QDateTime& timestamp, LOG_LEV level,
                                 CategoryId categoryId, const QString& msg) {
    loggedEntries.append({timestamp, level, categoryId, msg});
}

void TestJournal::cleanUpOldEntries(int keepDays) {
    Q_UNUSED(keepDays);
}

void TestJournal::loadCategoriesFromModel() {

}

void TestJournal::reset() {
    insertCategoryCallCount = 0;
    lastInsertCategoryName.clear();
    loggedEntries.clear();
    registeredCategories.clear();
    nextCategoryId = 1;
    // Важно: generalCategory должен быть заново установлен
    m_generalCat = 0;
}

// ==================== JournalAccessTest ====================
void JournalAccessTest::init() {
    JournalAccess::setJournal(nullptr);
}

void JournalAccessTest::testInitialState() {
    QVERIFY(!JournalAccess::isReady());
    QVERIFY(JournalAccess::journal() == nullptr);
}

void JournalAccessTest::testSetAndGet() {
    TestJournal tj;
    JournalAccess::setJournal(&tj);
    QVERIFY(JournalAccess::isReady());
    QCOMPARE(JournalAccess::journal(), &tj);
}

void JournalAccessTest::testSetNull() {
    TestJournal tj;
    JournalAccess::setJournal(&tj);
    QVERIFY(JournalAccess::isReady());
    JournalAccess::setJournal(nullptr);
    QVERIFY(!JournalAccess::isReady());
    QVERIFY(JournalAccess::journal() == nullptr);
}

// ==================== JournalBaseTest ====================
void JournalBaseTest::init() {
    m_testJournal = new TestJournal();
    JournalAccess::setJournal(m_testJournal);
}

void JournalBaseTest::cleanup() {
    JournalAccess::setJournal(nullptr);
    delete m_testJournal;
    m_testJournal = nullptr;
}

void JournalBaseTest::testGeneralCategory() {
    QVERIFY(m_testJournal->generalCategory() != 0);
    QString name = m_testJournal->categoryName(m_testJournal->generalCategory());
    QCOMPARE(name, QString("General"));
    // При создании TestJournal должен был вызвать insertCategory для "General"
    QCOMPARE(m_testJournal->insertCategoryCallCount, 1);
    QCOMPARE(m_testJournal->lastInsertCategoryName, QString("General"));
}

void JournalBaseTest::testRegisterCategory() {
    QString categoryName = "TestCategory";
    CategoryId id1 = m_testJournal->registerCategory(categoryName);

    // Проверяем, что insertCategory был вызван
    QCOMPARE(m_testJournal->insertCategoryCallCount, 2); // 1 от General + 1 от TestCategory
    QCOMPARE(m_testJournal->lastInsertCategoryName, categoryName);
    QVERIFY(id1 != 0);

    // Повторная регистрация той же категории
    CategoryId id2 = m_testJournal->registerCategory(categoryName);
    QCOMPARE(id1, id2);
    // insertCategory не должен вызываться повторно
    QCOMPARE(m_testJournal->insertCategoryCallCount, 2);
}

void JournalBaseTest::testCategoryIdAndName() {
    QString categoryName = "AnotherCategory";
    CategoryId id = m_testJournal->registerCategory(categoryName);

    // Проверяем categoryId
    QCOMPARE(m_testJournal->categoryId(categoryName), id);
    QCOMPARE(m_testJournal->categoryId("Unknown"), (CategoryId)0);

    // Проверяем categoryName
    QCOMPARE(m_testJournal->categoryName(id), categoryName);
    QCOMPARE(m_testJournal->categoryName(999), QString());
}

void JournalBaseTest::testLoggingWithCategoryId() {
    QString categoryName = "LogCategory";
    CategoryId categoryId = m_testJournal->registerCategory(categoryName);
    m_testJournal->reset(); // сбрасываем счетчики, но кэш остаётся

    QString msg = "Test message";
    m_testJournal->info(categoryId, msg);

    QCOMPARE(m_testJournal->loggedEntries.size(), 1);
    const auto& entry = m_testJournal->loggedEntries.first();
    QCOMPARE(entry.level, LOG_LEV::INFO);
    QCOMPARE(entry.category, categoryId);
    QCOMPARE(entry.message, msg);
    // Проверяем, что временная метка примерно корректна (в пределах 5 секунд)
    QVERIFY(qAbs(entry.timestamp.secsTo(QDateTime::currentDateTime())) < 5);

    // Проверяем, что insertLogEntry был вызван ровно один раз
    QCOMPARE(m_testJournal->insertCategoryCallCount, 0); // регистраций не было
}

void JournalBaseTest::testLoggingWithoutCategory() {
    m_testJournal->reset();
    QString msg = "General message";
    m_testJournal->info(msg);

    QCOMPARE(m_testJournal->loggedEntries.size(), 1);
    const auto& entry = m_testJournal->loggedEntries.first();
    QCOMPARE(entry.level, LOG_LEV::INFO);
    QCOMPARE(entry.category, m_testJournal->generalCategory());
    QCOMPARE(entry.message, msg);
}

void JournalBaseTest::testLoggingWithCategoryName() {
    m_testJournal->reset();

    QString categoryName = "DynamicCategory";
    QString msg = "Dynamic message";

    // Первый вызов с новым именем категории
    m_testJournal->info(categoryName, msg);

    // Проверяем, что insertCategory был вызван
    QCOMPARE(m_testJournal->insertCategoryCallCount, 1);
    QCOMPARE(m_testJournal->lastInsertCategoryName, categoryName);

    // Должна быть запись в логе
    QCOMPARE(m_testJournal->loggedEntries.size(), 1);
    CategoryId expectedId = m_testJournal->registeredCategories[categoryName];
    QCOMPARE(m_testJournal->loggedEntries.first().category, expectedId);
    QCOMPARE(m_testJournal->loggedEntries.first().message, msg);

    // Второй вызов с тем же именем
    m_testJournal->info(categoryName, "Another message");
    QCOMPARE(m_testJournal->insertCategoryCallCount, 1); // insertCategory не вызывался повторно
    QCOMPARE(m_testJournal->loggedEntries.size(), 2);
    QCOMPARE(m_testJournal->loggedEntries[1].category, expectedId);
}

void JournalBaseTest::testCacheConsistency() {
    QString categoryName = "CachedCategory";
    CategoryId id = m_testJournal->registerCategory(categoryName);
    m_testJournal->reset(); // сброс счетчиков, кэш остаётся

    // Проверяем, что categoryId не вызывает insertCategory
    CategoryId id2 = m_testJournal->categoryId(categoryName);
    QCOMPARE(id, id2);
    QCOMPARE(m_testJournal->insertCategoryCallCount, 0);

    // Проверяем, что categoryName также использует кэш
    QString name = m_testJournal->categoryName(id);
    QCOMPARE(name, categoryName);

    // Логирование через имя тоже использует кэш (не вызывает insertCategory)
    m_testJournal->info(categoryName, "cached message");
    QCOMPARE(m_testJournal->insertCategoryCallCount, 0);
}

