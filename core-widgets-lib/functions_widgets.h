#ifndef FUNCTIONSWIDGETS_H
#define FUNCTIONSWIDGETS_H

#include <QDialog>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "core-widgets-lib-global.h"

CORE_WIDGETS_LIB_EXPORT void delegate_selection_paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              QStyleOptionViewItem &opt,
                              const QModelIndex &index);

CORE_WIDGETS_LIB_EXPORT void center_dialog_mainwindow(QDialog* dialog);
CORE_WIDGETS_LIB_EXPORT void center_dialog(QDialog *dialog);

#endif // FUNCTIONSWIDGETS_H
