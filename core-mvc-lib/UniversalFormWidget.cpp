#include "UniversalFormWidget.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QComboBox>
#include <QTableView>
#include <QDialog>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QAbstractItemModel>

UniversalFormWidget::UniversalFormWidget(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
}

void UniversalFormWidget::setModel(QAbstractItemModel *model)
{
    if (!model) return;
    m_model = model;
    if (m_mapper) m_mapper->deleteLater();
    m_mapper = new QDataWidgetMapper(this);
    m_mapper->setModel(model);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit); // будем обновлять вручную
}

void UniversalFormWidget::setTableView(QTableView *view)
{
    m_tableView = view;
}

// Новый основной метод настройки колонок с указанием типа редактора
void UniversalFormWidget::setupColumns(const QVector<ColumnSetup> &columns)
{
    // Очистка старых виджетов
    qDeleteAll(m_displayWidgets);
    m_displayWidgets.clear();
    m_delegates.clear();
    m_editorTypes.clear();
    m_columnSetups.clear();

    QFormLayout *layout = qobject_cast<QFormLayout*>(this->layout());
    if (!layout) return;
    while (layout->rowCount() > 0)
        layout->removeRow(0);

    for (const auto &cs : columns) {
        if (!cs.visible) continue;

        m_columnSetups[cs.column] = cs;
        m_editorTypes[cs.column] = cs.editorType;
        m_visibleFlags[cs.column] = true;

        QWidget *editor = nullptr;

        switch (cs.editorType) {
        case SpinBox: {
            auto *spin = new QSpinBox;
            spin->setMinimum(cs.min);
            spin->setMaximum(cs.max);
            spin->setSingleStep(cs.singleStep);
            // мгновенное обновление модели при изменении значения
            connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, col = cs.column](int val) {
                if (m_currentRow >= 0 && m_model)
                    m_model->setData(m_model->index(m_currentRow, col), val);
            });
            editor = spin;
            break;
        }
        case DoubleSpinBox: {
            auto *dspin = new QDoubleSpinBox;
            dspin->setMinimum(cs.min);
            dspin->setMaximum(cs.max);
            dspin->setDecimals(cs.decimals);
            dspin->setSingleStep(cs.singleStep);
            connect(dspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, col = cs.column](double val) {
                if (m_currentRow >= 0 && m_model)
                    m_model->setData(m_model->index(m_currentRow, col), val);
            });
            editor = dspin;
            break;
        }
        case CheckBox: {
            auto *cb = new QCheckBox;
            connect(cb, &QCheckBox::toggled, this, [this, col = cs.column](bool checked) {
                if (m_currentRow >= 0 && m_model)
                    m_model->setData(m_model->index(m_currentRow, col), checked ? 1 : 0);
            });
            editor = cb;
            break;
        }
        case DateEdit: {
            auto *de = new QDateEdit;
            de->setCalendarPopup(true);
            connect(de, &QDateEdit::dateChanged, this, [this, col = cs.column](const QDate &date) {
                if (m_currentRow >= 0 && m_model)
                    m_model->setData(m_model->index(m_currentRow, col), date);
            });
            editor = de;
            break;
        }
        case ComboBox: {
            auto *cmb = new QComboBox;
            cmb->setEditable(true); // можно сделать editable для relation
            connect(cmb, &QComboBox::currentTextChanged, this, [this, col = cs.column](const QString &text) {
                if (m_currentRow >= 0 && m_model)
                    m_model->setData(m_model->index(m_currentRow, col), text);
            });
            editor = cmb;
            break;
        }
        case ReadOnlyLineEdit:
        default: {
            auto *le = new QLineEdit;
            le->setReadOnly(true);
            le->installEventFilter(this);
            le->setProperty("column", cs.column);
            editor = le;
            break;
        }
        }

        layout->addRow(cs.label + ":", editor);
        m_displayWidgets[cs.column] = editor;
    }

    // Привязываем QDataWidgetMapper только для ReadOnlyLineEdit полей
    if (m_model && m_mapper) {
        for (auto it = m_displayWidgets.begin(); it != m_displayWidgets.end(); ++it) {
            int col = it.key();
            if (m_editorTypes[col] == ReadOnlyLineEdit) {
                auto *le = qobject_cast<QLineEdit*>(it.value());
                if (le)
                    m_mapper->addMapping(le, col, "text");
            }
        }
    }
}

void UniversalFormWidget::setDelegates(const QMap<int, QAbstractItemDelegate*> &delegates)
{
    m_delegates = delegates;
}

void UniversalFormWidget::setCurrentRow(int row)
{
    if (!m_model || row < 0 || row >= m_model->rowCount()) return;

    m_currentRow = row;

    // Блокируем сигналы виджетов, чтобы не вызвать повторное сохранение
    for (auto it = m_displayWidgets.begin(); it != m_displayWidgets.end(); ++it) {
        it.value()->blockSignals(true);
    }

    // Обновляем ReadOnly поля через маппер
    if (m_mapper) {
        m_mapper->setCurrentIndex(row);
    }

    // Обновляем встроенные редакторы данными из модели
    for (auto it = m_displayWidgets.begin(); it != m_displayWidgets.end(); ++it) {
        int col = it.key();
        QWidget *w = it.value();
        EditorType type = m_editorTypes.value(col, ReadOnlyLineEdit);
        QModelIndex idx = m_model->index(row, col);
        QVariant val = idx.data(Qt::EditRole);

        switch (type) {
        case SpinBox:
            qobject_cast<QSpinBox*>(w)->setValue(val.toInt());
            break;
        case DoubleSpinBox:
            qobject_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble());
            break;
        case CheckBox:
            qobject_cast<QCheckBox*>(w)->setChecked(val.toInt() == 1);
            break;
        case DateEdit:
            qobject_cast<QDateEdit*>(w)->setDate(val.toDate());
            break;
        case ComboBox:
            qobject_cast<QComboBox*>(w)->setCurrentText(val.toString());
            break;
        case ReadOnlyLineEdit: {
            QLineEdit *le = qobject_cast<QLineEdit*>(w);
            if (le) {
                // Для relation используем DisplayRole
                QString text = idx.data(Qt::DisplayRole).toString();
                le->setText(text);
            }
            break;
        }
        }
    }

    // Разблокируем сигналы
    for (auto it = m_displayWidgets.begin(); it != m_displayWidgets.end(); ++it) {
        it.value()->blockSignals(false);
    }
}

void UniversalFormWidget::setColumnVisible(int column, bool visible)
{
    if (!m_displayWidgets.contains(column)) return;
    QWidget *w = m_displayWidgets[column];
    w->setVisible(visible);
    m_visibleFlags[column] = visible;
}

bool UniversalFormWidget::isColumnVisible(int column) const
{
    return m_visibleFlags.value(column, false);
}

bool UniversalFormWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (w && w->property("column").isValid()) {
            int col = w->property("column").toInt();
            // Открываем редактор через делегат только для ReadOnlyLineEdit
            if (m_editorTypes.value(col) == ReadOnlyLineEdit) {
                openDelegateEditor(col);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void UniversalFormWidget::openDelegateEditor(int column)
{
    // ... оставьте старую реализацию для диалоговых делегатов ...
    // (без изменений по сравнению с предыдущей версией, только убрать лишние типы)
    // Приведу сокращённо:
    if (m_currentRow < 0 || !m_model || !m_displayWidgets.contains(column))
        return;

    QAbstractItemDelegate *delegate = m_delegates.value(column, nullptr);
    if (!delegate) return;

    QModelIndex index = m_model->index(m_currentRow, column);
    QWidget *editorParent = m_tableView ? qobject_cast<QWidget*>(m_tableView) : this;
    QWidget *editor = delegate->createEditor(editorParent, QStyleOptionViewItem(), index);
    if (!editor) return;

    delegate->setEditorData(editor, index);

    if (QDialog *dialog = qobject_cast<QDialog*>(editor)) {
        if (m_tableView)
            dialog->setProperty("tableView", QVariant::fromValue<QWidget*>(m_tableView));
        if (dialog->exec() == QDialog::Accepted) {
            delegate->setModelData(editor, const_cast<QAbstractItemModel*>(m_model),
                                   const_cast<QModelIndex&>(index));
            updateDisplay();
        }
        editor->deleteLater();
        return;
    }

    QDialog *wrapper = new QDialog(m_tableView);
    wrapper->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    wrapper->setAttribute(Qt::WA_DeleteOnClose);
    if (m_tableView)
        wrapper->setProperty("tableView", QVariant::fromValue<QWidget*>(m_tableView));

    QVBoxLayout *lay = new QVBoxLayout(wrapper);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(editor);
    wrapper->setLayout(lay);
    wrapper->adjustSize();

    QWidget *displayWidget = m_displayWidgets[column];
    QPoint globalPos = displayWidget->mapToGlobal(QPoint(0, displayWidget->height()));
    wrapper->move(globalPos);
    wrapper->show();

    connect(wrapper, &QDialog::finished, this, [this, delegate, index, editor, wrapper](int) {
        if (editor)
            delegate->setModelData(editor, const_cast<QAbstractItemModel*>(m_model),
                                   const_cast<QModelIndex&>(index));
        updateDisplay();
        wrapper->deleteLater();
    });

    if (auto date = qobject_cast<QDateEdit*>(editor))
        connect(date, &QDateEdit::editingFinished, wrapper, &QDialog::accept);
    else if (auto dsb = qobject_cast<QDoubleSpinBox*>(editor))
        connect(dsb, &QDoubleSpinBox::editingFinished, wrapper, &QDialog::accept);
    else if (auto sb = qobject_cast<QSpinBox*>(editor))
        connect(sb, &QSpinBox::editingFinished, wrapper, &QDialog::accept);
    else if (auto cb = qobject_cast<QCheckBox*>(editor))
        connect(cb, &QCheckBox::toggled, wrapper, &QDialog::accept);
}

void UniversalFormWidget::updateDisplay()
{
    setCurrentRow(m_currentRow);
}
