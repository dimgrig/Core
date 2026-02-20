#include "BaseSqlRelationalTableModel.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>

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

            if (this->record().field(col).name().toLower().contains("date")) {
                //date field
                //qCDebug(logDebug) << "date field";
                //conditions << QString("%1 = %2").arg(fieldName).arg(value);
                conditions << QString("%1 %2").arg(fieldName).arg(value);
            } else {
                // Формируем условие в зависимости от типа данных
                if (type == QVariant::Int || type == QVariant::Double) {
                    conditions << QString("%1 = %2").arg(fieldName).arg(escapedValue);
                } else {
                    //string or date
                    conditions << QString("%1 LIKE '%2'").arg(fieldName).arg(escapedValue); //'%%2%'
                }
            }
        }
    }

    // Объединяем условия через AND
    QString filter = conditions.join(" AND ");

    //qCDebug(logDebug) << column << text << filter << this->record().field(column).type();

    // Применяем фильтр
    this->setFilter(filter);
    this->select();
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
