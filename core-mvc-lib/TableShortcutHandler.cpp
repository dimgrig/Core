#include "TableShortcutHandler.h"

#include <QApplication>
#include <QShortcut>

TableShortcutHandler::TableShortcutHandler(QWidget* parent) :
    QObject(parent)
{

}

TableShortcutHandler& TableShortcutHandler::instance() {
    static TableShortcutHandler inst;
    return inst;
}

void TableShortcutHandler::registerTableView(QTableView* tableView, bool enableShortcuts) {
    _tables.append(tableView);
    if (enableShortcuts) {
        this->setupShortcuts(tableView);
    }
}

void TableShortcutHandler::setupShortcuts(QTableView* _tableView) {
    auto tableView = qobject_cast<BaseEditTableView*>(_tableView);
    if (tableView != nullptr) {
        //tableView->setFocusPolicy(Qt::StrongFocus);

        QShortcut* copyShortcut = new QShortcut(QKeySequence::Copy, tableView, SLOT(slotCopy()), nullptr, Qt::WidgetWithChildrenShortcut);
        QShortcut* pasteShortcut = new QShortcut(QKeySequence::Paste, tableView, SLOT(slotPaste()), nullptr, Qt::WidgetWithChildrenShortcut);
        QShortcut* cutShortcut = new QShortcut(QKeySequence::Cut, tableView, SLOT(slotCut()), nullptr, Qt::WidgetWithChildrenShortcut);
        QShortcut* deleteShortcut = new QShortcut(QKeySequence::Delete, tableView, SLOT(slotDelete()), nullptr, Qt::WidgetWithChildrenShortcut);
        QShortcut* selectAllShortcut = new QShortcut(QKeySequence::SelectAll, tableView, SLOT(slotSelectAll()), nullptr, Qt::WidgetWithChildrenShortcut);
        QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, tableView, SLOT(slotSave()), nullptr, Qt::WidgetWithChildrenShortcut);
    }
}
