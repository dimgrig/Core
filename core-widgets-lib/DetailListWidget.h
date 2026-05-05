#ifndef DETAILLISTWIDGET_H
#define DETAILLISTWIDGET_H

#include <QWidget>
#include <QListView>
#include <QStackedWidget>
#include <QSplitter>
#include <QAbstractItemModel>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT DetailListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DetailListWidget(QString name, QWidget *parent = nullptr);

    // Установить модель для списка (отображается одна колонка)
    void setModel(QAbstractItemModel *model, int displayColumn = 0);

    // Добавить страницу панели для определённой строки
    void insertPanel(int row, QWidget *panel);
    void removePanel(int row);

    // Очистить все панели
    void clearPanels();

    // Получить текущую страницу / индекс
    QWidget *currentPanel() const;

    int currentRow() const;
    void setCurrentRow(int row);

signals:
    void currentRowChanged(int row);

private slots:
    void onSelectionChanged();

private:
    QListView *m_listView;
    QStackedWidget *m_stack;
    QAbstractItemModel *m_model = nullptr;
    int m_displayColumn = 0;
};

#endif // DETAILLISTWIDGET_H
