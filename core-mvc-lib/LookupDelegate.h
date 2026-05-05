#ifndef LOOKUPDELEGATE_H
#define LOOKUPDELEGATE_H

#include <functional>

#include <QStyledItemDelegate>
#include <QStringList>
#include <QTableView>

#include "BaseEditTableView.h"
#include "BaseSqlRelationalTableModel.h"

struct LookupConfig {
    // --- Таблица-справочник ---
    QString lookupTableName;                   // например "users"
    QString keyColumnName;                     // "id"
    QString displayColumn;                     // "fullname"
    QStringList columnNames;                   // имена всех столбцов модели
    QStringList columnHeaders;                 // заголовки в том же порядке
    QString sortColumn;                        // по какому столбцу сортировать (по умолчанию keyColumn)
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    // --- Основная таблица (где вызывается делегат) ---
    // Имя таблицы
    QString mainTableName;
    // Имя столбца в основной таблице, куда пишется id выбранной записи справочника
    QString mainTableForeignKeyColumn;
    // Имя первичного ключа основной таблицы (для прямого SQL, если понадобится)
    QString mainTablePrimaryKeyColumn = "id";

    // --- Внешний вид и поведение ---
    QSize dialogMinimumSize = {640, 480};
    QString windowTitle = "Выбор записи";
    std::function<bool()> isEditable;                   // разрешено ли редактировать справочник
    bool useFilterHeader = true;

    using DelegateCreator = std::function<QStyledItemDelegate*(QObject*)>;
    QMap<int, DelegateCreator> columnDelegates;

    // Дополнительный колбэк, который вызывается после успешного сохранения справочника
    std::function<void()> onAfterSave;        // например, обновить что-то ещё

    // Фабрика представления: если задана, делегат использует её для создания QTableView*
    // По умолчанию создаётся обычный QTableView.
    std::function<BaseEditTableView*(QWidget*)> viewFactory;

    // При необходимости: делегаты для колонок справочника (ключ – индекс колонки или имя)
    std::function<void(QTableView*)> customViewSetup; // можно добавить настройку view извне
};

class LookupEditorContext : public QObject {
    Q_OBJECT
public:
    explicit LookupEditorContext(QObject *parent = nullptr) : QObject(parent) {};

    BaseSqlRelationalTableModel* main_model = nullptr;
    BaseSqlRelationalTableModel* model = nullptr;
    QTableView* view = nullptr;
    int initId = -1;
    // ... другие нужные поля
};

class LookupDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit LookupDelegate(const LookupConfig &config, QObject *parent = nullptr);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
    void direct_sql_update(QAbstractItemModel* model, const QModelIndex& index, int id) const;

private:
    LookupConfig m_config;
};

#endif
