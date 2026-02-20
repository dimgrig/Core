#ifndef COMPLETER_H
#define COMPLETER_H

#include <QCompleter>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractItemView>

#include "core-mvc-lib-global.h"

template<typename T>
CORE_MVC_LIB_EXPORT void HimikCompleter(
    T* widget,
    QAbstractItemModel* model,
    int column,
    Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive,
    QCompleter::CompletionMode mode = QCompleter::PopupCompletion
    ) {
    static_assert(std::is_same_v<T, QLineEdit> || std::is_same_v<T, QComboBox>, "T must be QLineEdit or QComboBox");

    QCompleter* completer = new QCompleter(model, widget);
    completer->setCompletionColumn(column);
    completer->setCaseSensitivity(sensitivity);
    completer->setCompletionMode(mode);

    if constexpr (std::is_same_v<T, QComboBox>) {
        completer->setFilterMode(Qt::MatchContains);
        widget->setEditable(true);
    }

    widget->setCompleter(completer);
}

template<typename T>
CORE_MVC_LIB_EXPORT void setupCompleter(
    T* widget,
    const QStringList& items,
    Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive
    ) {
    static_assert(std::is_same_v<T, QLineEdit> || std::is_same_v<T, QComboBox>, "T must be QLineEdit or QComboBox");

    QCompleter* completer = new QCompleter(items, widget);
    completer->setCaseSensitivity(sensitivity);

    if constexpr (std::is_same_v<T, QComboBox>) {
        widget->addItems(items);
        widget->setEditable(true);
    }

    widget->setCompleter(completer);
}

#endif // COMPLETER_H
