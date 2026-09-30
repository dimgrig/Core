#ifndef ROUNDEDLABEL_H
#define ROUNDEDLABEL_H

#include <QLabel>
#include <QColor>
#include <QPainter>

#include "core-widgets-lib-global.h"

// QLabel с закруглённым фоном, без использования QSS.
// padding задаётся через setMargin(), цвет текста — через QPalette::WindowText.
class CORE_WIDGETS_LIB_EXPORT RoundedLabel : public QLabel {
    Q_OBJECT
public:
    explicit RoundedLabel(QWidget* parent = nullptr);

    void setBackgroundColor(const QColor& color);

    void setBorderRadius(int r);

    void setTextColor(const QColor& c) { m_textColor = c; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor m_bgColor;
    QColor m_textColor;
    int m_radius;
};

#endif // ROUNDEDLABEL_H
