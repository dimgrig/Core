#include "LookupDelegate.h"

#include <QCoreApplication>
#include <QDialog>
#include <QProgressDialog>
#include <QTimer>
#include <QVBoxLayout>

#include <QSqlError>
#include <QSqlQuery>

#include "aqp.h"
#include "functions.h"

#include "BaseEditTableView.h"
#include "EditBar.h"
#include "FilterHeader.h"
#include "TableShortcutHandler.h"

#include "Categories.h"
#include "JournalAccess.h"

LookupDelegate::LookupDelegate(const LookupConfig &config, QObject *parent)
    : QStyledItemDelegate(parent), m_config(config)
{}

QWidget* LookupDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    auto *editor = new QDialog(parent);
    editor->setWindowTitle(m_config.windowTitle);
    editor->setMinimumSize(m_config.dialogMinimumSize);

    QTimer::singleShot(0, editor, [editor]() {
        center_dialog(editor);
    });

    auto *ctx = new LookupEditorContext(editor); // дочерний объект

    {
        QTableView* table_view = nullptr;
        QWidget* current_parent = parent;
        while (current_parent && !table_view) {
            table_view = qobject_cast<QTableView*>(current_parent);
            current_parent = current_parent->parentWidget();
        }
        if (table_view != nullptr) {
            auto main_model = dynamic_cast<BaseSqlRelationalTableModel*>(table_view->model());
            if (main_model) {
                ctx->main_model = main_model;
            }
        }
    }

    // --- Создаём модель справочника ---
    auto *model = new BaseSqlRelationalTableModel(editor);
    model->setTable(m_config.lookupTableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setSort(model->fieldIndex(m_config.sortColumn), m_config.sortOrder);
    model->select();

    // Прогресс-диалог (по желанию)
    QProgressDialog progress("Загрузка...", "Отмена", 0, 0, parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(model->rowCount());
    progress.show();

    while (model->canFetchMore()) {
        model->fetchMore();
        QCoreApplication::processEvents(); // Для обработки событий UI

        progress.setValue(model->rowCount());
        if (progress.wasCanceled()) {
            break;
        }
    }

    // --- Настраиваем представление ---
    BaseEditTableView* view = nullptr;
    if (m_config.viewFactory) {
        view = m_config.viewFactory(editor);
    }

    if (view) {
        view->setObjectName(m_config.lookupTableName + "_table_view");
        view->setModel(model);

        for (int i = 0; i < m_config.columnHeaders.size(); ++i)
            model->setHeaderData(i, Qt::Horizontal, m_config.columnHeaders[i]);

        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->resizeColumnsToContents();
        view->setEditTriggers(QAbstractItemView::AllEditTriggers);
        view->horizontalHeader()->setStretchLastSection(true);

        view->setDragEnabled(true);
        view->setDefaultDropAction(Qt::MoveAction);//not necessary
        view->setDragDropMode(QAbstractItemView::InternalMove);
        view->setDropIndicatorShown(true);
        view->setAcceptDrops(true);
        view->viewport()->setAcceptDrops(true);
        view->setDragDropOverwriteMode(false);

        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

        model->set_editable(m_config.isEditable ? m_config.isEditable() : false);
        view->set_editable(m_config.isEditable ? m_config.isEditable() : false);

        ctx->model = model;
        ctx->view  = view;

        // Фильтр (опционально)
        if (m_config.useFilterHeader) {
            static int lookupDelegateCnt = 0;
            auto *filterHeader = new FilterHeader("LookupDelegate_" + QString::number(lookupDelegateCnt++), view);
            view->setHorizontalHeader(filterHeader);

            disconnect(view->horizontalHeader(), &QHeaderView::customContextMenuRequested, view, &BaseTableView::slotHeaderCustomMenuRequested);
            connect(view->horizontalHeader(), &QHeaderView::customContextMenuRequested, view, &BaseEditTableView::slotHeaderCustomMenuRequested);

            connect(filterHeader, &FilterHeader::filterChanged, [=](int col, const QString& text) {
                auto m = qobject_cast<BaseSqlRelationalTableModel*>(view->model());
                if (m) m->filter_changed(col, text);
            });
        }

        // --- Панель инструментов (аналог EditBar) ---
        EditBar *edit_bar = new EditBar(editor);
        edit_bar->setButtonVisible(EditBar::Button::Add, true);
        edit_bar->setButtonVisible(EditBar::Button::AddBefore, false);
        edit_bar->setButtonVisible(EditBar::Button::AddAfter, false);
        edit_bar->setButtonVisible(EditBar::Button::Copy, true);
        edit_bar->setButtonVisible(EditBar::Button::Cut, true);
        edit_bar->setButtonVisible(EditBar::Button::Paste, true);
        edit_bar->setButtonVisible(EditBar::Button::PasteBefore, false);
        edit_bar->setButtonVisible(EditBar::Button::PasteAfter, false);
        edit_bar->setButtonVisible(EditBar::Button::Delete, true);
        edit_bar->setButtonVisible(EditBar::Button::Save, true);
        edit_bar->setButtonVisible(EditBar::Button::Print, false);
        edit_bar->setButtonVisible(EditBar::Button::Export, false);
        edit_bar->setButtonVisible(EditBar::Button::Cancel, true);
        edit_bar->setButtonVisible(EditBar::Button::Ok, true);
        edit_bar->setButtonVisible(EditBar::Button::Custom1, false);
        edit_bar->setButtonVisible(EditBar::Button::Custom2, false);
        edit_bar->setButtonVisible(EditBar::Button::Custom3, false);
        edit_bar->disable_separators();
        edit_bar->tb_save->setEnabled(false);

        connect(model, &BaseSqlRelationalTableModel::modified, edit_bar->tb_save, &QToolButton::setEnabled);

        auto save_l = [ctx]() {if (!ctx->model) return;
            ctx->model->database().transaction();
            if (ctx->model->submitAll()) {
                ctx->model->database().commit();
                ctx->model->select();
                ctx->model->set_modified(false);
            } else {
                ctx->model->database().rollback();
                ctx->model->set_modified(true);
                AQP::warning(nullptr, "Ошибка", "Не удалось сохранить изменения.",
                             ctx->model->lastError().text());
            }
        };
        {
            connect(model, &BaseSqlRelationalTableModel::modified, [=](bool modified) {
                edit_bar->tb_save->setEnabled(modified);
            });
            connect(edit_bar->tb_save, &QToolButton::clicked, [=](){
                model->set_modified(false);
            });
            connect(edit_bar->tb_add, &QToolButton::clicked, view, &BaseEditTableView::slotAddAfter);
            connect(edit_bar->tb_copy, &QToolButton::clicked, view, &BaseEditTableView::slotCopy);
            connect(edit_bar->tb_cut, &QToolButton::clicked, view, &BaseEditTableView::slotCut);
            connect(edit_bar->tb_paste, &QToolButton::clicked, view, &BaseEditTableView::slotPaste);
            connect(edit_bar->tb_save, &QToolButton::clicked, save_l); //this, &UserDelegate::save
            connect(edit_bar->tb_delete, &QToolButton::clicked, view, &BaseEditTableView::slotDelete);
            connect(edit_bar->tb_cancel, &QToolButton::clicked, editor, &QDialog::reject);
            connect(edit_bar->tb_ok, &QToolButton::clicked, editor, &QDialog::accept);
        }

        connect(view, &QTableView::doubleClicked, editor, &QDialog::accept);
        connect(view, &BaseEditTableView::save, save_l);

        // Установка начальной строки (по значению текущей ячейки)
        QString displayValue = index.data(Qt::DisplayRole).toString();
        // Ищем запись с таким же отображаемым значением
        QSqlQuery query;
        query.prepare(QString("SELECT %1 FROM %2 WHERE %3 = ?")
                          .arg(m_config.keyColumnName, m_config.lookupTableName, m_config.displayColumn));
        query.addBindValue(displayValue);
        if (query.exec() && query.next()) {
            int id = query.value(0).toInt();
            //m_initId = id;
            ctx->initId = id;
            // ищем строку с этим id в модели
            int keyCol = model->fieldIndex(m_config.keyColumnName);
            for (int row = 0; row < model->rowCount(); ++row) {
                if (model->data(model->index(row, keyCol)).toInt() == id) {
                    view->selectRow(row);
                    break;
                }
            }
        }

        for (auto it = m_config.columnDelegates.constBegin(); it != m_config.columnDelegates.constEnd(); ++it) {
            QStyledItemDelegate* del = it.value()(view);
            view->setItemDelegateForColumn(it.key(), del);
        }

        // Если есть внешняя функция настройки представления – вызываем
        if (m_config.customViewSetup)
            m_config.customViewSetup(view);

        tsh.registerTableView(view);

        QVBoxLayout *layout = new QVBoxLayout(editor);
        layout->addWidget(edit_bar);
        layout->addWidget(view);

        view->view_repaint();
    }

    return editor;
}

void LookupDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDialog *dlg = qobject_cast<QDialog*>(editor);
    if (!dlg) return;

    auto *ctx = dlg->findChild<LookupEditorContext*>();
    if (!ctx) return;

    if (dlg->result() != QDialog::Accepted) {
        // проверяем, существует ли ещё изначальный id (если был)
        if (ctx->initId > 0) {
            QSqlQuery q;
            q.prepare(QString("SELECT 1 FROM %1 WHERE %2 = ?")
                          .arg(m_config.lookupTableName, m_config.keyColumnName));
            q.addBindValue(ctx->initId);
            if (q.exec() && q.next()) return; // id жив, оставляем как было
        }
        // иначе идём дальше и сохраняем текущее выделение
    }

    int selectedId = -1;
    if (ctx->view && ctx->model && ctx->view->selectionModel()->hasSelection()) {
        int row = ctx->view->currentIndex().row();
        int keyCol = ctx->model->fieldIndex(m_config.keyColumnName);
        selectedId = ctx->model->data(ctx->model->index(row, keyCol)).toInt();

        ctx->model->updateRelations();

        bool res = model->setData(index, selectedId > 0 ? QVariant(selectedId) : QVariant(), Qt::EditRole); //return true
        if (!res) {
            direct_sql_update(model, index, selectedId);
        }
    } else {
        bool res = model->setData(index, "", Qt::EditRole);
        Q_UNUSED(res);
    }

    if (ctx->main_model != nullptr) {
        ctx->main_model->database().transaction();
        if (ctx->main_model->submitAll()) {
            ctx->main_model->database().commit();
            ctx->main_model->select();
            ctx->main_model->set_modified(false);
            //qCDebug(logDebug()) << "Изменения успешно сохранены!";
        } else {
            ctx->main_model->database().rollback();
            ctx->main_model->set_modified(true);
            AQP::warning(nullptr, tr("Внимание"), tr("Не удалось сохранить изменения."), QString(m_config.mainTableName) + " " +  ctx->main_model->lastError().text(), 0);
            if (auto* j = JournalAccess::journal())
                j->warning(Category::Database, QString("Не удалось сохранить изменения: " + QString(m_config.mainTableName) + " " +  ctx->main_model->lastError().text()));
        }
    }

    if (m_config.onAfterSave)
        m_config.onAfterSave();
}

// Прямое обновление через SQL
void LookupDelegate::direct_sql_update(QAbstractItemModel* model, const QModelIndex& index, int selectedId) const {
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    try {
        // 1. Получаем ID основной записи
        int main_row_id = model->data(model->index(index.row(), 0)).toInt();

        // 2. Обновляем запись в основной таблице
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE " + m_config.mainTableName + " SET " + m_config.mainTableForeignKeyColumn + " = ? WHERE id = ?");
        updateQuery.addBindValue(selectedId);
        updateQuery.addBindValue(main_row_id);

        if (!updateQuery.exec()) {
            throw std::runtime_error(updateQuery.lastError().text().toStdString());
        }

        // 3. Фиксируем изменения
        if (!db.commit()) {
            throw std::runtime_error("Commit failed");
        }

        // 4. Принудительно обновляем модель
        if (auto sqlModel = dynamic_cast<QSqlTableModel*>(model)) {
            sqlModel->select();
            emit model->dataChanged(index, index);
        }

        //qCDebug(logDebug) << "Direct SQL update successful";
    } catch (const std::exception& e) {
        db.rollback();
        if (auto* j = JournalAccess::journal())
            j->warning(Category::Database, QString("Не удалось сохранить изменения: %1").arg(e.what()));
    }
}

void LookupDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index); // или ваша дополнительная логика
}

// // Прямой UPDATE
// QString mainTable = ctx->model->tableName(); // нужно, чтобы модель знала имя таблицы
// int mainRowId = model->data(model->index(index.row(),
//                                          ctx->model->fieldIndex(m_config.mainTablePrimaryKeyColumn))).toInt();
// QSqlQuery update;
// update.prepare(QString("UPDATE %1 SET %2 = ? WHERE %3 = ?")
//                    .arg(mainTable, m_config.mainTableForeignKeyColumn, m_config.mainTablePrimaryKeyColumn));
// update.addBindValue(selectedId);
// update.addBindValue(mainRowId);
// if (update.exec()) {
//     // Обновить модель
//     if (auto sqlModel = qobject_cast<QSqlTableModel*>(model)) {
//         sqlModel->select();
//     }
// }
