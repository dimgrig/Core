#ifndef TABLESHORTCUTHANDLER_H
#define TABLESHORTCUTHANDLER_H

#include <QObject>
#include "BaseEditTableView.h"

#include "core-mvc-lib-global.h"

#define tsh TableShortcutHandler::instance()

class CORE_MVC_LIB_EXPORT TableShortcutHandler : public QObject {
    Q_OBJECT
public:
    static TableShortcutHandler& instance() {
        static TableShortcutHandler inst;
        return inst;
    }

    void registerTableView(QTableView* tableView, bool enableShortcuts = true);
private slots:

private:
    explicit TableShortcutHandler(QWidget* parent = nullptr);
    TableShortcutHandler(const TableShortcutHandler&) = delete;
    TableShortcutHandler& operator=(const TableShortcutHandler&) = delete;
    ~TableShortcutHandler() {
        //std::cout << "Config::~Config()" << std::endl;
    }

    void setupShortcuts(QTableView* tableView);

    QList<QTableView*> _tables;
};

#endif // TABLESHORTCUTHANDLER_H
