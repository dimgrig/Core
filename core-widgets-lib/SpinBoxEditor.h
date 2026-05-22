#ifndef SPINBOXEDITOR_H
#define SPINBOXEDITOR_H

#include <QItemEditorFactory>
#include <QItemEditorCreatorBase>
#include <QLineEdit>
#include <QSpinBox>
#include <QVariant>

#include "core-widgets-lib-global.h"

// Создаем пользовательский редактор
class CORE_WIDGETS_LIB_EXPORT SpinBoxEditor : public QSpinBox {
    Q_OBJECT
public:
    SpinBoxEditor(QWidget *parent = nullptr) : QSpinBox(parent) {
        setMinimum(0);
        setMaximum(100);
    }
};

// Создаем редактор через QItemEditorCreatorBase
class SpinBoxEditorCreator : public QItemEditorCreatorBase {
public:
    QWidget *createWidget(QWidget *parent) const override {
        return new SpinBoxEditor(parent);
    }

    QByteArray valuePropertyName() const override {
        return QByteArray("value");
    }
};

//// Настраиваем QItemEditorFactory
//QItemEditorFactory *factory = new QItemEditorFactory;
//factory->registerEditor(QVariant::Int, new SpinBoxEditorCreator);
//QItemEditorFactory::setDefaultFactory(factory);

//https://doc.qt.io/qt-6/qitemeditorfactory.html


#endif // SPINBOXEDITOR_H
