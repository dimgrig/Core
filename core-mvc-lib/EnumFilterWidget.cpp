#include "EnumFilterWidget.h"

#include <QVBoxLayout>
#include <QRegularExpression>

EnumFilterWidget::EnumFilterWidget(const QVector<QPair<QString, QVariant>> &items,
                                   const QString &currentFilter,
                                   QWidget *parent)
    : QWidget(parent), m_listWidget(new QListWidget(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_listWidget);

    buildUi(items);
    applyFilter(currentFilter);
}

void EnumFilterWidget::buildUi(const QVector<QPair<QString, QVariant>> &items)
{
    for (const auto &[text, value] : items) {
        auto *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, value);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_listWidget->addItem(item);
    }
}

void EnumFilterWidget::applyFilter(const QString &filter)
{
    // Ожидаем формат "IN (val1, val2, ...)"
    static QRegularExpression inRe(R"(IN\s*\(([^)]+)\))", QRegularExpression::CaseInsensitiveOption);
    auto match = inRe.match(filter);
    if (!match.hasMatch())
        return;

    const QStringList strValues = match.captured(1).split(',', Qt::SkipEmptyParts);

    // Создаём множество строковых представлений выбранных значений
    QSet<QString> selectedSet;
    for (const QString &s : strValues) {
        bool ok;
        int intVal = s.trimmed().toInt(&ok);
        if (ok)
            selectedSet.insert(QString::number(intVal));
        else
            selectedSet.insert(s.trimmed());
    }

    // Отмечаем элементы, у которых строковое представление data совпадает с одним из выбранных
    for (int i = 0; i < m_listWidget->count(); ++i) {
        auto *item = m_listWidget->item(i);
        QString itemValue = item->data(Qt::UserRole).toString();
        item->setCheckState(selectedSet.contains(itemValue) ? Qt::Checked : Qt::Unchecked);
    }
}

QVariantList EnumFilterWidget::selectedValues() const
{
    QVariantList values;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        auto *item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked)
            values.append(item->data(Qt::UserRole));
    }
    return values;
}
