#ifndef LISTEDITDELEGATE_H
#define LISTEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QStringList>

class ListEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ListEditDelegate(QObject *parent = nullptr);

    // Создание редактора (открывает диалог)
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    // Установка данных из модели в редактор
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    // Сохранение данных из редактора в модель
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    // Размер редактора (не используется, т.к. редактор — диалог)
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

#endif // LISTEDITDELEGATE_H
