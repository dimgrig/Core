#ifndef JOURNALBASE_H
#define JOURNALBASE_H

#include <QString>
#include <QHash>
#include <QMutex>
#include <QDateTime>

#include "Logger.h"

#include "core-journal-lib-global.h"

class CORE_JOURNAL_LIB_EXPORT JournalBase {
public:
    virtual ~JournalBase();

    // Публичный API для логирования с указанием категории
    void debug(CategoryId categoryId, const QString& msg);
    void info(CategoryId categoryId, const QString& msg);
    void warning(CategoryId categoryId, const QString& msg);
    void critical(CategoryId categoryId, const QString& msg);
    void fatal(CategoryId categoryId, const QString& msg);

    void debug(const QString& categoryName, const QString& msg);
    void info(const QString& categoryName, const QString& msg);
    void warning(const QString& categoryName, const QString& msg);
    void critical(const QString& categoryName, const QString& msg);
    void fatal(const QString& categoryName, const QString& msg);

    // Версии без категории (используют категорию по умолчанию "General")
    void debug(const QString& msg);
    void info(const QString& msg);
    void warning(const QString& msg);
    void critical(const QString& msg);
    void fatal(const QString& msg);

    // Управление категориями
    CategoryId registerCategory(const QString& name);
    CategoryId categoryId(const QString& name) const;
    QString categoryName(CategoryId id) const;

    // Категория по умолчанию
    CategoryId generalCategory() const { return m_generalCat; }

protected:
    JournalBase();  // protected конструктор – абстрактный класс

    // Чисто виртуальные методы для работы с БД (реализуются в наследниках)
    virtual CategoryId insertCategory(const QString& name) = 0;
    virtual void insertLogEntry(const QDateTime& timestamp, LOG_LEV level,
                                CategoryId categoryId, const QString& msg) = 0;
    virtual void loadCategoriesFromModel() = 0;

    // Запись в файл через Logger (может использоваться наследниками)
    void logToFile(LOG_LEV level, const QString& fullMsg);

    // Общий метод логирования
    void log(LOG_LEV level, CategoryId categoryId, const QString& msg);

    void addCategoryToCache(CategoryId id, const QString& name);

    // Кэш категорий (потокобезопасный)
    mutable QMutex m_cacheMutex;
    QHash<QString, CategoryId> m_categoryCache;
    QHash<CategoryId, QString> m_reverseCache;
    CategoryId m_generalCat = 0;  // будет установлено наследником
};

#endif // JOURNALBASE_H
