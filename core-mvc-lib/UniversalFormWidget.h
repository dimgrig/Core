#ifndef UNIVERSALFORMWIDGET_H
#define UNIVERSALFORMWIDGET_H

#include <QWidget>
#include <QDataWidgetMapper>
#include <QMap>
#include <QVector>
#include <QAbstractItemDelegate>

class QTableView;

class UniversalFormWidget : public QWidget
{
    Q_OBJECT
public:
    enum EditorType {
        ReadOnlyLineEdit,   // только чтение, двойной клик вызывает делегат
        SpinBox,            // QSpinBox
        DoubleSpinBox,      // QDoubleSpinBox
        CheckBox,           // QCheckBox
        DateEdit,           // QDateEdit
        ComboBox            // QComboBox (для relation можно наполнять отдельно)
    };

    // Структура для описания одной колонки
    struct ColumnSetup {
        int column;
        QString label;
        EditorType editorType = ReadOnlyLineEdit; // по умолчанию – через делегат
        bool visible = true;
        // Для SpinBox/DoubleSpinBox можно задать min/max/step (опционально)
        int min = 0, max = 1000, decimals = 0;
        double singleStep = 1.0;
    };

    explicit UniversalFormWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    void setTableView(QTableView *view);

    // Основной метод: настройка колонок с указанием типа редактора
    void setupColumns(const QVector<ColumnSetup> &columns);

    // Установка делегатов для колонок (для ReadOnlyLineEdit)
    void setDelegates(const QMap<int, QAbstractItemDelegate*> &delegates);

    void setCurrentRow(int row);

    void setColumnVisible(int column, bool visible);
    bool isColumnVisible(int column) const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupSimpleEditor(int col, EditorType type, const ColumnSetup &setup);
    void openDelegateEditor(int column);
    void updateDisplay();

    QDataWidgetMapper *m_mapper = nullptr;
    QTableView *m_tableView = nullptr;
    QAbstractItemModel *m_model = nullptr;

    QMap<int, ColumnSetup> m_columnSetups;       // колонка → настройка
    QMap<int, QAbstractItemDelegate*> m_delegates; // колонка → делегат (для ReadOnlyLineEdit)
    QMap<int, QWidget*> m_displayWidgets;        // колонка → виджет (любого типа)
    QMap<int, EditorType> m_editorTypes;         // колонка → тип редактора
    QMap<int, bool> m_visibleFlags;
    int m_currentRow = -1;
};

#endif // UNIVERSALFORMWIDGET_H
