#include "WidgetResizeMixin.h"

WidgetResizeMixin::WidgetResizeMixin(QWidget *parent) :
    QWidget(parent)
{

}

WidgetResizeMixin::~WidgetResizeMixin() {

}

void WidgetResizeMixin::resizeEvent(QResizeEvent *event) {
    emit resize_event(0, 0);
    QWidget::resizeEvent(event);
}

void WidgetResizeMixin::paintEvent(QPaintEvent *event) {

    QWidget::paintEvent(event);
}
