#ifndef FOCUSWATCHER_H
#define FOCUSWATCHER_H

#include <QObject>
#include <QEvent>

#include <core-lib/Logger.h>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT FocusWatcher : public QObject {
Q_OBJECT
public:
    explicit FocusWatcher(QObject* parent = nullptr) :
        QObject(parent),
        _parent(parent)
    {
        if (parent)
            parent->installEventFilter(this);
    };

    virtual bool eventFilter(QObject *obj, QEvent *event) override {
        Q_UNUSED(obj)
        if (event->type() == QEvent::FocusIn) {
            qCDebug(logDebug()) << _parent->objectName() << "FocusIn";
            emit focusChanged(true);
        } else if (event->type() == QEvent::FocusOut) {
            qCDebug(logDebug()) << _parent->objectName() << "FocusOut";
            emit focusChanged(false);
        }

        return false;
    }

Q_SIGNALS:
   void focusChanged(bool in);
private:
   QObject* _parent;
};

#endif // FOCUSWATCHER_H
