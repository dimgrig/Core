#ifndef ROTATEDLABEL_H
#define ROTATEDLABEL_H

#include <QLabel>

#include "core-widgets-lib-global.h"

class CORE_WIDGETS_LIB_EXPORT RotatedLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(int rotation READ rotation WRITE setRotation)
public:
    explicit RotatedLabel(QWidget *parent = nullptr);

    int rotation() const { return _rotation; }
    void setRotation(int degrees);   // 0, 90, 180, 270

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int _rotation = 270; // по умолчанию вертикально
    int calcOptimalFontSize() const;
};

#endif
