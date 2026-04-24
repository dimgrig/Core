#include "ValueListDelegate.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QHeaderView>
#include <QListWidget>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QSpinBox>

#include "CoreConfig.h"

QList<double> str_parse_to_list(const QString &value_str) {
    QList<double> values;
    QStringList value_sl = value_str.split(ELEMENTS_DELIMETER);
    QLocale locale(QLocale::Russian);
    for (const auto &value : value_sl) {
        bool ok;
        double d = locale.toDouble(value, &ok);
        if (ok) {
            values.append(d);
        } else {
            values.append(value.toDouble());
        }
    }
    return values;
}

QString join_list_to_string(const QList<double> &values) {
    QStringList value_sl;
    QLocale locale(QLocale::Russian);
    for (double value : values) {
        value_sl << locale.toString(value, 'f', 2); // формат с двумя знаками после запятой
    }
    return value_sl.join(ELEMENTS_DELIMETER);
}

// Реализация диалога
ValueListDialog::ValueListDialog(QStringList header_sl, QWidget *parent)
    : QDialog(parent)
{
    this->setMinimumSize(QSize(200, 400));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Создаем таблицу с одной колонкой
    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(1);
    tableWidget->setHorizontalHeaderLabels(header_sl);
    tableWidget->verticalHeader()->hide();
    tableWidget->horizontalHeader()->setStretchLastSection(true);

    // Кнопки управления
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("+ Добавить", this);
    removeButton = new QPushButton("- Удалить", this);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    mainLayout->addWidget(tableWidget);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(buttonBox);

    connect(addButton, &QPushButton::clicked, this, &ValueListDialog::addItem);
    connect(removeButton, &QPushButton::clicked, this, &ValueListDialog::removeItem);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ValueListDialog::setValues(const QList<double> &values)
{
    tableWidget->setRowCount(0);
    QLocale locale(QLocale::Russian);
    for (double value : values) {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);

        QDoubleSpinBox *spinBox = new QDoubleSpinBox();
        spinBox->setLocale(locale);
        spinBox->setMinimum(0);
        spinBox->setMaximum(500);
        spinBox->setDecimals(2);
        spinBox->setSingleStep(0.01);
        spinBox->setValue(value);
        tableWidget->setCellWidget(row, 0, spinBox);
    }
}

QList<double> ValueListDialog::getValues() const
{
    QList<double> values;
    for (int i = 0; i < tableWidget->rowCount(); ++i) {
        QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(tableWidget->cellWidget(i, 0));
        if (spinBox) {
            values.append(spinBox->value());
        }
    }
    return values;
}

void ValueListDialog::addItem()
{
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);

    QDoubleSpinBox *spinBox = new QDoubleSpinBox();
    QLocale locale(QLocale::Russian);
    spinBox->setLocale(locale);
    spinBox->setMinimum(0);
    spinBox->setMaximum(500);
    spinBox->setDecimals(2);
    spinBox->setSingleStep(0.01);
    spinBox->setValue(0);
    tableWidget->setCellWidget(row, 0, spinBox);
    tableWidget->setCurrentCell(row, 0);
}

void ValueListDialog::removeItem()
{
    int row = tableWidget->currentRow();
    if (row >= 0) {
        tableWidget->removeRow(row);
    }
}

// Реализация делегата
ValueListDelegate::ValueListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ValueListDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &/*option*/,
                                            const QModelIndex &/*index*/) const
{
    ValueListDialog *dialog = new ValueListDialog({"Значения"}, parent);
    dialog->setModal(true);
    return dialog;
}

void ValueListDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const
{
    ValueListDialog *dialog = qobject_cast<ValueListDialog*>(editor);
    if (!dialog) return;

    QString str = index.data(Qt::EditRole).toString();
    QList<double> values = str_parse_to_list(str);
    dialog->setValues(values);
}

void ValueListDelegate::setModelData(QWidget *editor,
                                        QAbstractItemModel *model,
                                        const QModelIndex &index) const
{
    ValueListDialog *dialog = qobject_cast<ValueListDialog*>(editor);
    if (!dialog) return;

    if (dialog->result() == QDialog::Accepted) {
        QList<double> values = dialog->getValues();
        QString str = join_list_to_string(values);
        model->setData(index, str, Qt::EditRole);
    }
}
