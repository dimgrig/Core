#include "BaseEditTableView.h"

#include "aqp.h"
#include "Auth.h"
#include "Logger.h"

BaseEditTableView::BaseEditTableView(QWidget *parent) :
    BaseTableView(parent)
{
    // QShortcut *CtrlC = new QShortcut(this);
    // QShortcut *CtrlV = new QShortcut(this);
    // QShortcut *CtrlX = new QShortcut(this);
    // QShortcut *CtrlD = new QShortcut(this);
    // QShortcut *CtrlA = new QShortcut(this);
    // //QShortcut *CtrlP = new QShortcut(this);
    // QShortcut *Del = new QShortcut(this);
    // //QShortcut *CtrlS = new QShortcut(this);

    // CtrlC->setKey(Qt::CTRL + Qt::Key_C);
    // CtrlV->setKey(Qt::CTRL + Qt::Key_V);
    // CtrlX->setKey(Qt::CTRL + Qt::Key_X);
    // CtrlD->setKey(Qt::CTRL + Qt::Key_D);
    // CtrlA->setKey(Qt::CTRL + Qt::Key_A);
    // //CtrlP->setKey(Qt::CTRL + Qt::Key_P);
    // Del->setKey(Qt::Key_Delete);
    // //CtrlS->setKey(Qt::CTRL + Qt::Key_S);

    // connect(CtrlC, &QShortcut::activated, this, &BaseEditTableView::slotCopy);
    // connect(CtrlV, &QShortcut::activated, this, &BaseEditTableView::slotPaste);
    // connect(CtrlX, &QShortcut::activated, this, &BaseEditTableView::slotCut);
    // connect(CtrlD, &QShortcut::activated, this, &BaseEditTableView::slotDelete);
    // connect(CtrlA, &QShortcut::activated, this, &BaseEditTableView::slotSelectAll);
    // connect(Del, &QShortcut::activated, this, &BaseEditTableView::slotDelete);
    // //connect(CtrlS, &QShortcut::activated, this, &BaseEditTableView::slotSave);

    //ContextMenuPolicy CHANGE!!!
    connect(this, &BaseEditTableView::customContextMenuRequested, this, &BaseEditTableView::slotCustomMenuRequested);

    disconnect(this->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &BaseTableView::slotHeaderCustomMenuRequested);
    connect(this->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &BaseEditTableView::slotHeaderCustomMenuRequested);
}

BaseEditTableView::~BaseEditTableView()
{

}

QVariant BaseEditTableView::getDefaultValueForColumn(int column) {
    // Можно добавить логику для разных типов данных
    QAbstractItemModel *model = this->model();
    if (!model) return QVariant(0);

    // Пример: проверка типа данных столбца
    QModelIndex sampleIndex = model->index(0, column);
    if (!sampleIndex.isValid()) return QVariant();

    QVariant sampleData = model->data(sampleIndex);

    switch (sampleData.type()) {
    case QVariant::Bool:
        return false;
    case QVariant::String:
        return QString("");
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        return 0;
    case QVariant::Double:
        return 0.0;
    // moved to initializeNewRow
    // case QVariant::Date:
    //     return QDate::currentDate();
    // case QVariant::DateTime:
    //     return QDateTime::currentDateTime();
    // case QVariant::Time:
    //     return QTime::currentTime();
    default:
        return QVariant();
    }
}

void BaseEditTableView::fetch_model() const {
    if (!model()) return;

    int previousRowCount = -1;
    while (this->model()->canFetchMore(QModelIndex())) {
        int currentRowCount = model()->rowCount();
        // Break if row count doesn't increase (prevents infinite loop)
        if (currentRowCount == previousRowCount) {
            break;
        }
        previousRowCount = currentRowCount;
        model()->fetchMore(QModelIndex());
    }
}

bool BaseEditTableView::check_clipboard(const QAbstractItemModel * model, QVector<QStringList> &rows_v) {
    QString pasted_text = QApplication::clipboard()->text();
    QStringList pasted_text_rows;
    if (pasted_text.contains(ROW_DELIMETER)) {
        pasted_text_rows = pasted_text.split(ROW_DELIMETER);
    } else if (pasted_text.contains("\n")) {
        pasted_text_rows = pasted_text.split("\n");
    } else {
        pasted_text_rows << pasted_text; //possibly one row
    }

    for (const auto &row : pasted_text_rows) {
        QStringList cells = row.split(ELEMENTS_DELIMETER);
        rows_v.push_back(cells);
    }

    bool check_rows = true;

    for (const auto &row : rows_v) {
        if (row.size() != model->columnCount()) {
            check_rows = false;
            break;
        }
    }

    return check_rows;
}

void BaseEditTableView::slotCustomMenuRequested(QPoint pos)
{
    //    qCDebug(logDebug()) << "handleContextMenu";

    QItemSelectionModel *select = this->selectionModel();

    /* Создаем объект контекстного меню */
    QMenu * menu = new QMenu(this);

    //    select->hasSelection(); //check if has selection
    //    select->selectedRows(); // return selected row(s)
    //    select->selectedColumns(); // return selected column(s)
    if (select->hasSelection()) {
        //if (AuthUserRole_from_text(auth_user.user.get_level().c_str()) != Operator) {
            /* Создаём действия для контекстного меню */
            QAction * AddBefore = new QAction("Добавить до", this);
            QAction * AddAfter = new QAction("Добавить после", this);
            QAction * Copy = new QAction("Копировать", this);
            QAction * Cut = new QAction("Вырезать", this);
            QAction * Paste = new QAction("Вставить (заменить)", this);
            QAction * InsertBefore = new QAction("Вставить до", this);
            QAction * InsertAfter = new QAction("Вставить после", this);
            QAction * Delete = new QAction("Удалить", this);

            /* Подключаем СЛОТы обработчики для действий контекстного меню */
            connect(AddBefore, &QAction::triggered, this, &BaseEditTableView::slotAddBefore);
            connect(AddAfter, &QAction::triggered, this, &BaseEditTableView::slotAddAfter);
            connect(Copy, &QAction::triggered, this, &BaseEditTableView::slotCopy);
            connect(Cut, &QAction::triggered, this, &BaseEditTableView::slotCut);
            connect(Paste, &QAction::triggered, this, &BaseEditTableView::slotPaste);
            connect(InsertBefore, &QAction::triggered, this, &BaseEditTableView::slotInsertBefore);
            connect(InsertAfter, &QAction::triggered, this, &BaseEditTableView::slotInsertAfter);
            connect(Delete, &QAction::triggered, this, &BaseEditTableView::slotDelete);

            /* Устанавливаем действия в меню */
            menu->addAction(AddBefore);
            menu->addAction(AddAfter);
            menu->addAction(Copy);
            menu->addAction(Cut);
            QVector<QStringList> rows_v;
            bool check_rows = check_clipboard(this->model(), rows_v);
            if (check_rows) {
                menu->addAction(Paste);
                menu->addAction(InsertBefore);
                menu->addAction(InsertAfter);
            }
            menu->addAction(Delete);
        //}
    } else {
        //if (AuthUserRole_from_text(auth_user.user.get_level().c_str()) != Operator) {
            QAction * Add = new QAction("Добавить", this);
            connect(Add, &QAction::triggered, this, &BaseEditTableView::slotAddAfter);
            menu->addAction(Add);
        //}
    }

    // if (this->model()->rowCount() > 0) {
    //     this->add_print_export_actions(menu);
    // }

    /* Вызываем контекстное меню */
    if (!menu->actions().empty()) {
        menu->popup(this->mapToGlobal(pos));
    }
}

void BaseEditTableView::slotAddBefore() {
    //qCDebug(logDebug()) << "slotAddBefore fires";
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                     .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    QItemSelectionModel *select = this->selectionModel();
    if (select->hasSelection()) {

        QModelIndex index = select->currentIndex();
        if (!index.isValid())
            return;

        QModelIndexList indexes = select->selectedRows();
        QAbstractItemModel * model = const_cast<QAbstractItemModel*>(indexes.first().model());

        int row = indexes.first().row();
        model->insertRows(row, 1);

        this->initializeNewRow(row);
        this->slotAdd(row);
    }

    this->view_repaint();
}

void BaseEditTableView::slotAddAfter() {
    //qCDebug(logDebug()) << "slotAddAfter fires";
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    bool clear_selection = false;
    QItemSelectionModel *select = this->selectionModel();

    this->fetch_model();

    if (!select->hasSelection()) {    
        this->selectRow(this->model()->rowCount() - 1);
        clear_selection = true;
    }
    if (select->hasSelection()) {

        QModelIndex index = select->currentIndex();
        if (!index.isValid())
            return;

        QModelIndexList indexes = select->selectedRows();
        int row = indexes.first().row() + 1; //////////// +1 ////////////
        this->model()->insertRows(row, 1);

        this->initializeNewRow(row);
        this->slotAdd(row);
    } else {
        int row = this->model()->rowCount();
        this->model()->insertRows(row, 1);

        this->initializeNewRow(row);
        this->slotAdd(row);
    }
    if (clear_selection) {
        this->selectionModel()->clear();
    }

    this->view_repaint();
}

void BaseEditTableView::slotCopy() {
    //qCDebug(logDebug()) << "slotCopy fires";

    QItemSelectionModel *select = this->selectionModel();
    if (select->hasSelection()) {
        //qCDebug(logDebug()) << select->selectedRows();

        //        qCDebug(logDebug()) << select->selectedColumns().size() << this->model()->columnCount();
        //        if (select->selectedColumns().size() == this->model()->columnCount()) {
        copy_to_clipboard(select);
        //        }
    }

    //qCDebug(logDebug()) << QApplication::clipboard()->text();
    //emit Copy();
}

void BaseEditTableView::slotCut() {
    //qCDebug(logDebug()) << "slotCut fires";
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    QItemSelectionModel *select = this->selectionModel();
    if (select->hasSelection()) {
        this->slotCopy();
        this->_confrim_deletion = false;
        this->slotDelete();
        this->_confrim_deletion = true;
    }

    this->view_repaint();
}

void BaseEditTableView::slotPaste() {
    //qCDebug(logDebug()) << "slotPaste fires";
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    this->fetch_model();

    QItemSelectionModel *select = this->selectionModel();
    if (select->hasSelection()) {
        //qCDebug(logDebug()) << select->selectedRows();

        QModelIndex index = select->currentIndex();
        if (!index.isValid())
            return;

        QModelIndexList indexes = select->selectedRows();
        QVector<QStringList> rows_v;
        bool check_rows = check_clipboard(this->model(), rows_v);

        if (check_rows) {
            int row = indexes.first().row();
            if ((row + rows_v.size()) <= this->model()->rowCount()) {
                ;
            } else {
                qCDebug(logDebug) << row + rows_v.size() - this->model()->rowCount();
                this->model()->insertRows(this->model()->rowCount(), row + rows_v.size() - this->model()->rowCount());
                // volatile int rows_to_delete = this->model()->rowCount() - row;
                // this->model()->removeRows(row, this->model()->rowCount() - row);
                // qCDebug(logDebug) << row << rows_to_delete << this->model()->rowCount();
                // this->model()->insertRows(this->model()->rowCount(), row + rows_v.size() - this->model()->rowCount());
            }

            this->slotInsert(row, rows_v);
        }
    } else {
        QVector<QStringList> rows_v;
        bool check_rows = check_clipboard(this->model(), rows_v);

        if (check_rows) {
            int row = this->model()->rowCount();
            if ((row + rows_v.size()) <= this->model()->rowCount()) {
                ;
            } else {
                this->model()->insertRows(this->model()->rowCount(), row + rows_v.size() - this->model()->rowCount());
            }

            this->slotInsert(row, rows_v);
        }
    }
    this->view_repaint();
}

bool BaseEditTableView::isIdColumn(int column) const {
    QAbstractItemModel *model = this->model();
    if (!model) return false;

    QString header = model->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
    return header.contains("id", Qt::CaseInsensitive) ||
           header.contains("Id", Qt::CaseInsensitive) ||
           header.contains("ID", Qt::CaseInsensitive) ||
           _auto_increment_columns.contains(column);
}

void BaseEditTableView::initializeNewRow(int row) {
    QAbstractItemModel *model = this->model();
    if (!model) return;

    int columns = model->columnCount();
    for (int col = 0; col < columns; ++col) {
        if (isIdColumn(col)) {
            continue;
        }

        QString header = model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();

        if (header.contains("Дата", Qt::CaseInsensitive)) {
            QModelIndex index = model->index(row, col);
            QVariant defaultValue = QDate::currentDate();
            model->setData(index, defaultValue);

            continue;
        }

        QModelIndex index = model->index(row, col);
        QVariant defaultValue = this->getDefaultValueForColumn(col);
        model->setData(index, defaultValue);
    }
}

void BaseEditTableView::slotInsert(int row, const QVector<QStringList> &rows_v) {
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    for (const auto &_row : rows_v) {
        int col = 0;
        for (const auto &cell : _row) {
            if (!isIdColumn(col)) {
                this->model()->setData(this->model()->index(row, col), cell, Qt::EditRole);
            }
            ++col;
        }
        ++row;
    }
    this->view_repaint();
}

void BaseEditTableView::slotInsertBefore() {
    //qCDebug(logDebug()) << "slotInsertBefore fires";
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    QItemSelectionModel *select = this->selectionModel();
    if (select->hasSelection()) {
        //qCDebug(logDebug()) << select->selectedRows();

        QModelIndex index = select->currentIndex();
        if (!index.isValid())
            return;

        QModelIndexList indexes = select->selectedRows();
        QVector<QStringList> rows_v;
        bool check_rows = check_clipboard(this->model(), rows_v);

        if (check_rows) {
            int row = indexes.first().row();
            this->model()->insertRows(row, rows_v.size());

            this->slotInsert(row, rows_v);
        }
    }
    this->view_repaint();
}

void BaseEditTableView::slotInsertAfter() {
    //qCDebug(logDebug()) << "slotInsertBefore fires";
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    QItemSelectionModel *select = this->selectionModel();
    if (select->hasSelection()) {
        //qCDebug(logDebug()) << select->selectedRows();

        QModelIndex index = select->currentIndex();
        if (!index.isValid())
            return;

        QModelIndexList indexes = select->selectedRows();
        QVector<QStringList> rows_v;
        bool check_rows = check_clipboard(this->model(), rows_v);

        if (check_rows) {
            int row = indexes.first().row() + 1; ////////////// +1 ///////////////
            this->model()->insertRows(row, rows_v.size());
            this->slotInsert(row, rows_v);
        }
    }
    this->view_repaint();
}

void BaseEditTableView::slotSelectAll() {
    this->selectAll();
}

void BaseEditTableView::slotSave() {
    if (!_is_editable) {
        AQP::warning(this, tr("Внимание"), tr("Редактирование недоступно."),
                     QString(tr("Пользователь %1 имеет уровень доступа %2"))
                         .arg(auth_user.user.name).arg(auth_user.user.get_level().c_str()), 0, 10);
        return;
    }

    emit this->save();
}

//void CustomTableView::CopySlot()
//{
//    //this->setValue(value);
//}

void BaseEditTableView::contextMenuEvent(QContextMenuEvent *event){
    //qDebug() << "contextMenuEvent override";

    QTimer::singleShot(100, this, &BaseEditTableView::onTimerTimeout);

    QTableView::contextMenuEvent(event);
}

void BaseEditTableView::onTimerTimeout()
{
    qDebug() << "onTimerTimeout fires";
    QMenu * menu = this->findChild<QMenu*>("qt_edit_menu");

    if (menu) {
        qDebug() << "menu found";
        QAction * first_action = menu->actions().at(0);

        /* Создаём действия для контекстного меню */
        QAction * ActionCopy = new QAction("Копировать", this);

        /* Подключаем СЛОТы обработчики для действий контекстного меню */
        connect(ActionCopy, &QAction::triggered, this, &BaseEditTableView::slotCopy);

        menu->insertAction(first_action, ActionCopy);
        menu->insertSeparator(first_action);
    }
}


