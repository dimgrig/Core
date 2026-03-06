#include "BaseSqlRelationalTableModel.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>

#include <QRegularExpression>

#include "aqp.h"
#include "Logger.h"

BaseSqlRelationalTableModel::BaseSqlRelationalTableModel(QObject *parent)
    : QSqlRelationalTableModel(parent)
{
    _parent = parent;

    connect(this, &QAbstractTableModel::dataChanged, this, &BaseSqlRelationalTableModel::data_changed);
    connect(this, &QAbstractTableModel::rowsInserted, this, &BaseSqlRelationalTableModel::mark_as_modified);
    connect(this, &QAbstractTableModel::rowsRemoved, this, &BaseSqlRelationalTableModel::mark_as_modified);
}

bool BaseSqlRelationalTableModel::save() {
    this->database().transaction();
    if (this->submitAll()) {
        this->database().commit();
        this->select(); // Важно: обновляем данные модели
        this->set_modified(false);
        qCDebug(logDebug()) << "Изменения успешно сохранены!";

        emit saved();

        return true;
    } else {
        this->database().rollback();
        qCDebug(logDebug()) << "Ошибка, не удалось сохранить изменения: " + this->lastError().text();
        AQP::warning(nullptr, tr("Внимание"), tr("Не удалось сохранить изменения: ") + this->lastError().text(), 0, 10);
    }

    return false;
}

bool BaseSqlRelationalTableModel::setVirtualData(int, int, const QVariant&)
{
    return false;
}

QVariant BaseSqlRelationalTableModel::virtualData(int, int, int) const
{
    return QVariant();
}

int BaseSqlRelationalTableModel::findRowByKey(const QVariant& keyValue, const int column) const
{
    // По умолчанию предполагаем, что первичный ключ в колонке 0
    for (int row = 0; row < rowCount(); ++row) {
        if (data(index(row, column)) == keyValue)
            return row;
    }
    return -1;
}


bool BaseSqlRelationalTableModel::isVirtualColumn(int column) const {
    return column >= QSqlRelationalTableModel::columnCount() &&
           column < columnCount();
}

int BaseSqlRelationalTableModel::virtualColumnIndex(int column) const {
    return column - QSqlRelationalTableModel::columnCount();
}

int BaseSqlRelationalTableModel::virtualColumnAbsoluteIndex(int virtualColumnIndex) const {
    return QSqlRelationalTableModel::columnCount() + virtualColumnIndex;
}

void BaseSqlRelationalTableModel::set_modified(bool is_modified) {
    _is_modified = is_modified;
    emit modified(_is_modified);
}

void BaseSqlRelationalTableModel::set_editable(bool is_editable) {
    _is_editable = is_editable;
}

Qt::ItemFlags BaseSqlRelationalTableModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags default_flags = QSqlRelationalTableModel::flags(index);

    if (!is_editable()) {
        default_flags &= ~Qt::ItemIsEditable;
    }

    return default_flags;
}

void BaseSqlRelationalTableModel::filter_changed(int column, const QString &text) {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QSqlDriver* driver = this->database().driver();

    _filters[column] = text;

    // Список условий для фильтра
    QStringList conditions;

    for (auto it = _filters.begin(); it != _filters.end(); ++it) {
        int col = it.key();
        QString value = it.value();

        if (value.isEmpty()) continue;

        // Получаем имя поля в БД для столбца
        QString fieldName = this->record().fieldName(col);

        // Обработка реляционных столбцов
        if (this->relation(col).isValid()) {
            QSqlRelation relation = this->relation(col);
            QSqlTableModel* relation_model = this->relationModel(col);

            // Получаем имя поля в БД для столбца
            QString relation_field_name = this->record().fieldName(col);

            // Экранируем значение
            //QString relation_escaped_value = driver->escapeIdentifier(value, QSqlDriver::FieldName);
            QString relation_escaped_value = "%" + value + "%";

            // Ищем ID по отображаемому значению
            QString relation_filter = QString("%1 LIKE '%2'").arg(relation_field_name).arg(relation_escaped_value); //'%%2%'
            relation_model->setFilter(relation_filter);
            relation_model->select();

            QStringList ids;
            for (int i = 0; i < relation_model->rowCount(); ++i) {
                ids << relation_model->data(relation_model->index(i, 0)).toString();
            }

            if (!ids.isEmpty()) {
                //conditions << QString("%1 IN (%2)").arg(relation.indexColumn()).arg(ids.join(","));
                //conditions << QString("%1 IN (%2)").arg(relation_field_name).arg(ids.join(","));
                conditions << relation_filter;
            }

            //qCDebug(logDebug) << relation_filter << ids.size();
        } else {
            // Экранируем значение
            //QString escapedValue = driver->escapeIdentifier(value, QSqlDriver::FieldName);
            QString escapedValue = "%" + value + "%";

            QVariant::Type type = this->record().field(col).type();
            QString fieldNameLower = fieldName.toLower();

            if (fieldNameLower.contains("date")) {
                //date field
                //qCDebug(logDebug) << "date field";
                //conditions << QString("%1 = %2").arg(fieldName).arg(value);
                conditions << QString("%1 %2").arg(fieldName).arg(value);
            } else if (fieldNameLower.contains("timestamp")) {
                QString condition = buildTimestampCondition(fieldName, value);
                if (!condition.isEmpty())
                    conditions << condition;
            } else {
                // Формируем условие в зависимости от типа данных
                if (type == QVariant::Int) {
                    int num = value.toInt();
                    conditions << QString("%1 = %2").arg(fieldName).arg(num);
                } else if (type == QVariant::Double) {
                    double num = value.toDouble();
                    conditions << QString("%1 = %2").arg(fieldName).arg(num);
                } else {
                    //string or date
                    conditions << QString("%1 LIKE '%2'").arg(fieldName).arg(escapedValue); //'%%2%'
                }
            }
        }
    }

    // Объединяем условия через AND
    _fullFilter = conditions.join(" AND ");

    //qCDebug(logDebug) << column << text << filter << this->record().field(column).type();

    // Применяем фильтр
    qCDebug(logDebug()) << "setFilter start" << _fullFilter;
    this->setFilter(_fullFilter);
    this->select();
    qCDebug(logDebug()) << "setFilter end" << _fullFilter;

    emit filterChanged(_fullFilter);

    qCDebug(logDebug()) << "emit filterChanged end";

    QApplication::restoreOverrideCursor();
}

void BaseSqlRelationalTableModel::data_changed(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    if (!isVirtualColumn(topLeft.column())) {
        _is_modified = true;
        emit modified(_is_modified);
    }
}

void BaseSqlRelationalTableModel::mark_as_modified() {
    _is_modified = true;
    emit modified(_is_modified);
}

void BaseSqlRelationalTableModel::setRelation(int column, const QSqlRelation &relation) {
    this->_relation_column_s.insert(column);
    QSqlRelationalTableModel::setRelation(column, relation);
}

void BaseSqlRelationalTableModel::updateRelations() {
    for (int _column : this->_relation_column_s) {
        QSqlTableModel* relModel = relationModel(_column);

        // ПРИНУДИТЕЛЬНАЯ ПОЛНАЯ ЗАГРУЗКА
        relModel->select();
        while (relModel->canFetchMore()) {
            relModel->fetchMore();
        }
    }
}

void BaseSqlRelationalTableModel::updateRelations(int column) {
    for (int _column : this->_relation_column_s) {
        if (column == _column) {
            QSqlTableModel* relModel = relationModel(_column);

            // ПРИНУДИТЕЛЬНАЯ ПОЛНАЯ ЗАГРУЗКА
            relModel->select();
            while (relModel->canFetchMore()) {
                relModel->fetchMore();
            }
        }
    }
}

QString BaseSqlRelationalTableModel::buildTimestampCondition(const QString& fieldName, const QString& filterExpr)
{
    // Формат даты-времени, используемый в FilterHeader
    const QString datetimeFormat = DATE_TIME_FORMAT_WITHOUT_MS;

    // Если фильтр пустой – ничего не делаем
    if (filterExpr.isEmpty())
        return QString();

    // Парсим выражение
    QRegularExpression betweenRe("BETWEEN\\s+'([^']+)'\\s+AND\\s+'([^']+)'", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression geRe(">=\\s+'([^']+)'", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression leRe("<=\\s+'([^']+)'", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression eqRe("=\\s+'([^']+)'", QRegularExpression::CaseInsensitiveOption);

    QDateTime start, end;
    QString op;

    if (auto match = betweenRe.match(filterExpr); match.hasMatch()) {
        start = QDateTime::fromString(match.captured(1), datetimeFormat);
        end = QDateTime::fromString(match.captured(2), datetimeFormat);
        op = "BETWEEN";
    } else if (auto match = geRe.match(filterExpr); match.hasMatch()) {
        start = QDateTime::fromString(match.captured(1), datetimeFormat);
        end = QDateTime();
        op = ">=";
    } else if (auto match = leRe.match(filterExpr); match.hasMatch()) {
        start = QDateTime();
        end = QDateTime::fromString(match.captured(1), datetimeFormat);
        op = "<=";
    } else if (auto match = eqRe.match(filterExpr); match.hasMatch()) {
        start = end = QDateTime::fromString(match.captured(1), datetimeFormat);
        end = QDateTime();
        op = "=";
    } else {
        // Неизвестный формат – логируем и игнорируем
        start = QDateTime();
        end = QDateTime();
        qWarning() << "Unknown timestamp filter format:" << filterExpr;
        return QString();
    }

    // Преобразуем локальное время в UTC Unix timestamp
    // Предполагаем, что FilterHeader отправляет время в локальном поясе,
    // а в БД хранится UTC.
    auto toUtcSecs = [](const QDateTime& localDt) -> qint64 {
        if (!localDt.isValid()) return 0;
        QDateTime utcDt = localDt.toUTC();
        return utcDt.toSecsSinceEpoch();
    };

    _timestampOp = op;
    _startSecs = toUtcSecs(start);
    _endSecs = toUtcSecs(end);

    // Формируем условие в зависимости от оператора
    if (op == "BETWEEN") {
        if (start.isValid() && end.isValid()) {
            return QString("%1 BETWEEN %2 AND %3").arg(fieldName).arg(_startSecs).arg(_endSecs);
        }
    } else if (op == ">=") {
        if (start.isValid()) {
            return QString("%1 >= %2").arg(fieldName).arg(_startSecs);
        }
    } else if (op == "<=") {
        if (end.isValid()) {
            return QString("%1 <= %2").arg(fieldName).arg(_endSecs);
        }
    } else if (op == "=") {
        if (start.isValid()) {
            // Для точного равенства по дате-времени можно сделать диапазон
            // Например, если пользователь выбрал конкретный момент
            return QString("%1 = %2").arg(fieldName).arg(_startSecs);
        }
    }

    return QString();
}

bool BaseSqlRelationalTableModel::cleanUpOldEntries(const QString& table, const QString& timestamp, int keepDays) {
    QDateTime threshold = QDateTime::currentDateTimeUtc().addDays(-keepDays);
    volatile qint64 thresholdSecs = threshold.toSecsSinceEpoch();

    QSqlQuery query;
    query.prepare(QString("DELETE FROM %1 WHERE %2 < :threshold").arg(table).arg(timestamp));
    query.bindValue(":threshold", thresholdSecs);

    if (query.exec()) {
        return true;
    }

    qCDebug(logWarning()) << "Failed to clean up :" << this->lastError().text();

    return false;
}
