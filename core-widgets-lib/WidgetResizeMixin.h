#ifndef WIDGETRESIZEMIXIN_H
#define WIDGETRESIZEMIXIN_H

#include <QWidget>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT WidgetResizeMixin : public QWidget {
    Q_OBJECT
public:
    explicit WidgetResizeMixin(QWidget *parent = nullptr);
    ~WidgetResizeMixin();
signals:
    void resize_event(int w, int h);
private:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

#endif // WIDGETRESIZEMIXIN_H
