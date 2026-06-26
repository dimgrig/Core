#ifndef ENUMFILTERWIDGET_H
#define ENUMFILTERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QVariant>
#include <QVector>
#include <QPair>

class EnumFilterWidget : public QWidget {
    Q_OBJECT
public:
    // Параметр items: список {отображаемый_текст, значение_ключа}
    // currentFilter – текущая строка фильтра (например "IN (0,1,3)")
    explicit EnumFilterWidget(const QVector<QPair<QString, QVariant>> &items,
                              const QString &currentFilter,
                              QWidget *parent = nullptr);

    // Возвращает список выбранных значений (QVariant)
    QVariantList selectedValues() const;

private:
    void buildUi(const QVector<QPair<QString, QVariant>> &items);
    void applyFilter(const QString &filter);

    QListWidget *m_listWidget;
};



#endif // ENUMFILTERWIDGET_H
