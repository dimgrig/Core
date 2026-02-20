#ifndef BASEEDITTABLEVIEW_H
#define BASEEDITTABLEVIEW_H

#include "BaseTableView.h"

#include "core-mvc-lib-global.h"

class CORE_MVC_LIB_EXPORT BaseEditTableView : public BaseTableView {
    Q_OBJECT
public:
    BaseEditTableView(QWidget *parent = nullptr);
    ~BaseEditTableView();

    bool is_editable() const {return _is_editable;};
    void set_editable(bool is_editable = false) {_is_editable = is_editable;};

    bool confrim_deletion() const {return _confrim_deletion;};
    void set_confrim_deletion(bool confrim_deletion = false) {_confrim_deletion = confrim_deletion;};

    // Методы для работы с защищенными столбцами
    void setProtectedColumns(const QSet<int>& columns) {_protected_columns = columns;}
    void addProtectedColumn(int column) {_protected_columns.insert(column);}
    void removeProtectedColumn(int column) {_protected_columns.remove(column);}
    void clearProtectedColumns() {_protected_columns.clear();}
    QSet<int> protectedColumns() const {return _protected_columns;}

    // Методы для работы с автоинкрементными столбцами
    void setAutoIncrementColumns(const QSet<int>& columns) {_auto_increment_columns = columns;}
    void addAutoIncrementColumn(int column) {_auto_increment_columns.insert(column);}
    bool isAutoIncrementColumn(int column) const {return _auto_increment_columns.contains(column);}
signals:
    void save();
public slots:
    virtual void slotAdd(int row) = 0;
    virtual void slotDelete() = 0;
    virtual void slotCustomMenuRequested(QPoint pos) = 0;
    virtual void slotHeaderCustomMenuRequested(QPoint pos) = 0;

    void fetch_model() const;

    void slotAddBefore();
    void slotAddAfter();
    void slotCopy();
    void slotCut();
    void slotPaste();
    void slotInsertBefore();
    void slotInsertAfter();
    void slotSelectAll();
    void slotSave();
protected slots:
    bool isIdColumn(int column) const;
    virtual void initializeNewRow(int row);
    virtual void slotInsert(int row, const QVector<QStringList> &rows_v);

    void contextMenuEvent(QContextMenuEvent *event) override;
    void onTimerTimeout();

    bool check_clipboard(const QAbstractItemModel * model, QVector<QStringList> &rows_v);
protected:
    QVariant getDefaultValueForColumn(int column);

    bool _confrim_deletion = true;

    bool _is_editable = false;

    QSet<int> _protected_columns;  // Столбцы, значения которых нужно сохранять при вставке
    QSet<int> _auto_increment_columns;  // Столбцы с автоинкрементом

private:
};

#endif // BASEEDITTABLEVIEW_H
