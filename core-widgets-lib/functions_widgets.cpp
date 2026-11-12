#include "functions_widgets.h"

#include <QApplication>
#include <QScreen>
#include <QTableView>

void delegate_selection_paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              QStyleOptionViewItem &opt,
                              const QModelIndex &index) {
    const QWidget *widget = option.widget;
    QString str = qvariant_cast<QString>(index.data())+ "\n";
    str += QString::number(qvariant_cast<float>(index.data(Qt::UserRole)));
    opt.text = "";
    QStyle *style = widget ? widget->style() : QApplication::style();

    if (option.state & QStyle::State_Selected) {
        // White pen while selection
        painter->setPen(Qt::white);
        painter->setBrush(option.palette.highlightedText());
        // This call will take care to draw, dashed line while selecting
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    } else {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        painter->setPen(QPen(option.palette.color(QPalette::Text), 0));
#else
        painter->setPen(QPen(option.palette.foreground(), 0));
#endif
        painter->setBrush(qvariant_cast<QBrush>(index.data(Qt::ForegroundRole)));
    }
}

void center_dialog_mainwindow(QDialog* dialog) {
    // Подгоняем размер диалога под содержимое
    dialog->adjustSize();
    // Находим экран, на котором находится центр главного окна
    QWidget* mainWindow = qApp->activeWindow();
    if (mainWindow) {
        QPoint mainCenter = mainWindow->geometry().center();
        QScreen* screen = QGuiApplication::screenAt(mainCenter);
        if (!screen)
            screen = QGuiApplication::primaryScreen();
        // Вычисляем позицию, чтобы диалог оказался по центру этого экрана
        QRect screenRect = screen->geometry();
        QPoint dialogCenter = dialog->rect().center();
        QPoint newTopLeft = screenRect.center() - dialogCenter;
        dialog->move(newTopLeft);
    }
}

void center_dialog(QDialog* dialog) {
    if (!dialog) return;

    QTableView* table_view = nullptr;
    QWidget* current_parent = dynamic_cast<QWidget*>(dialog->parent());
    while (current_parent && !table_view) {
        table_view = qobject_cast<QTableView*>(current_parent);
        current_parent = current_parent->parentWidget();
    }

    // Если есть родительский виджет (например, таблица), центрируем относительно него
    if (table_view != nullptr) {
        QPoint parentCenter = table_view->mapToGlobal(table_view->rect().center());
        dialog->move(
            parentCenter.x() - dialog->width() / 2,
            parentCenter.y() - dialog->height() / 2
            );
    }
    // Иначе центрируем относительно курсора
    else {
        QPoint cursorPos = QCursor::pos();
        dialog->move(
            cursorPos.x() - dialog->width() / 2,
            cursorPos.y() - dialog->height() / 2
            );
    }
}
