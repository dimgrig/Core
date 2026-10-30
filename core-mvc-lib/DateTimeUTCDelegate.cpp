#include "DateTimeUTCDelegate.h"

#include <QDateTimeEdit>

#include "Logger.h"

DateTimeUTCDelegate::DateTimeUTCDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *DateTimeUTCDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QDateTimeEdit *editor = new QDateTimeEdit(parent);
    editor->setDisplayFormat(_format);
    editor->setCalendarPopup(true);      // Показывать календарь
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    editor->setTimeZone(QTimeZone("UTC"));
#else
    editor->setTimeSpec(Qt::UTC);
#endif
    return editor;
}

void DateTimeUTCDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDateTimeEdit *dateTimeEdit = static_cast<QDateTimeEdit*>(editor);

    // Из модели получаем число (Unix timestamp в секундах)
    qint64 timestamp = index.data(Qt::EditRole).toLongLong();
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp, QTimeZone("UTC"));
#else
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp, Qt::UTC);
#endif

    dateTimeEdit->setDateTime(dateTime);
}

void DateTimeUTCDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDateTimeEdit *dateTimeEdit = static_cast<QDateTimeEdit*>(editor);
    QDateTime dateTime = dateTimeEdit->dateTime();

    // Преобразуем обратно в Unix timestamp (секунды) и сохраняем в модель
    qint64 timestamp = dateTime.toSecsSinceEpoch();
    model->setData(index, timestamp, Qt::EditRole);
}

void DateTimeUTCDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
