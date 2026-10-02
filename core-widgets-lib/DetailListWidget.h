#ifndef DETAILLISTWIDGET_H
#define DETAILLISTWIDGET_H

#include <QWidget>
#include <QListView>
#include <QStackedWidget>
#include <QSplitter>
#include <QToolButton>
#include <QAbstractItemModel>

#include "core-widgets-lib-global.h"

#include <functional>

using DetailListWidgetPanelFactory = std::function<QWidget*(int row, const QModelIndex &index)>;

class CORE_WIDGETS_LIB_EXPORT DetailListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DetailListWidget(QString name, QWidget *parent = nullptr);

    // Установить модель для списка (отображается одна колонка)
    void setModel(QAbstractItemModel *model, DetailListWidgetPanelFactory factory, int idColumn, int displayColumn);

    // Добавить страницу панели для определённой строки
    void insertPanel(int row, QWidget *panel);
    void removePanel(int row);

    // Очистить все панели
    void clearPanels();

    // Получить текущую страницу / индекс
    QWidget *currentPanel() const;

    int currentRow() const;
    void setCurrentRow(int row);

    void updatePanels();

signals:
    void currentRowChanged(int row);
    void currentRowIdChanged(int id);

    void runTestRefreshSequenceRequested(int id);
    void runFromTestRequested(int id);
public slots:
    void triggerRepeat();
    void triggerStartFrom();
private slots:
    void triggerRepeatByRow(int row);
    void triggerStartFromByRow(int row);

    void onSelectionChanged();

    void onListViewContextMenu(const QPoint &pos);
private:
    int currentId() const;

    DetailListWidgetPanelFactory m_factory = nullptr;

    QListView           *m_listView     = nullptr;
    QStackedWidget      *m_stack        = nullptr;
    //QToolButton         *m_repeatButton = nullptr;
    //QToolButton         *m_startButton  = nullptr;
    QAbstractItemModel  *m_model        = nullptr;
    int m_idColumn = 0;
    int m_displayColumn = 0;
};

#endif // DETAILLISTWIDGET_H
