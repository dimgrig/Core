#ifndef MULTISELECTWIDGET_H
#define MULTISELECTWIDGET_H

#include <QHBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QWidget>

#include "FlowLayout.h"

class QLabel;
class QMenu;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class MultiSelectWidget : public QWidget {
    Q_OBJECT
public:
    explicit MultiSelectWidget(QWidget *parent = nullptr);
    ~MultiSelectWidget() override = default;

    // Установить доступные элементы (id -> отображаемое имя)
    void setItems(const QMap<int, QString> &items);

    // Получить / установить текущий выбор
    QVector<int> selectedIds() const;
    void setSelectedIds(const QVector<int> &ids);

signals:
    void selectionChanged(const QVector<int> &selectedIds);

private slots:
    void onAddButtonClicked();
    void onFilterTextChanged(const QString &text);
    void onMenuItemToggled(QListWidgetItem *item);
    void onRemoveChip(int id);

private:
    void rebuildChips();
    void populateMenu();
    QWidget* createChip(int id, const QString &name);

    QWidget *_flowContainer = nullptr;
    FlowLayout *_flowLayout = nullptr;

    QPushButton *_addButton = nullptr;
    QMenu *_popupMenu = nullptr;
    QLineEdit *_filterEdit = nullptr;
    QListWidget *_channelListWidget = nullptr;

    QMap<int, QString> _availableItems; // id -> name
    QSet<int> _selectedIds;
};

#endif // MULTISELECTWIDGET_H
