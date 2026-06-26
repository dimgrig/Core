#include "LogLevelDelegate.h"

#include <QLineEdit>

#include "CoreConfig.h"
#include "Logger.h"

LogLevelDelegate::LogLevelDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void LogLevelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Создаём копию опций, чтобы изменить фон
    QStyleOptionViewItem opt = option;

    // Получаем значение уровня из модели (Qt::EditRole хранит число)
    int levelValue = index.data(Qt::EditRole).toInt();
    LOG_LEV level = static_cast<LOG_LEV>(levelValue);

    // Выбираем цвет в зависимости от уровня
    QColor bgColor;
    switch (level) {
    case LOG_LEV::DEBUG:
        bgColor = LGRAY_COLOR;
        break;
    case LOG_LEV::INFO:
        bgColor = LG_COLOR;
        break;
    case LOG_LEV::WARNING:
        bgColor = LY_COLOR;
        break;
    case LOG_LEV::CRITICAL:
        bgColor = _LR_COLOR;
        break;
    case LOG_LEV::FATAL:
        bgColor = DR_COLOR;
        break;
    default:
        break;
    }

    painter->setBackgroundMode(Qt::TransparentMode);
    // Если цвет задан, устанавливаем его как фоновую кисть
    if (bgColor.isValid()) {
        painter->fillRect(option.rect, bgColor);
    }

    // Вызываем базовую отрисовку с изменёнными опциями
    QStyledItemDelegate::paint(painter, opt, index);
}

QWidget *LogLevelDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);

    return editor;
}

void LogLevelDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int level = index.model()->data(index, Qt::EditRole).toInt();
    QLineEdit *le = static_cast<QLineEdit*>(editor);

    le->setText(logLevelToString(static_cast<LOG_LEV>(level)));
}

void LogLevelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *le = static_cast<QLineEdit*>(editor);

    QString level = le->text();

    model->setData(index, static_cast<int>(logLevelFromString(level)), Qt::EditRole);
}

void LogLevelDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
