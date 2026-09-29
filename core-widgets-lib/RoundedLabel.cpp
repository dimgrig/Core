#include "RoundedLabel.h"

RoundedLabel::RoundedLabel(QWidget* parent)
    : QLabel(parent), m_radius(3) {
    setAutoFillBackground(false);   // фон рисуем сами
}

void RoundedLabel::setBackgroundColor(const QColor& color) {
    if (m_bgColor == color) return;
    m_bgColor = color;
    update();
}

void RoundedLabel::setBorderRadius(int r) {
    if (m_radius == r) return;
    m_radius = r;
    update();
}

void RoundedLabel::paintEvent(QPaintEvent* event) {
    if (m_bgColor.isValid()) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(m_bgColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), m_radius, m_radius);
    }
    if (m_textColor.isValid()) {
        QPainter p(this);
        p.setPen(m_textColor);
        p.setFont(font());
        p.drawText(rect().adjusted(2,2,-2,-2), alignment(), text());
        return;   // не вызываем QLabel::paintEvent
    }
    // текст и рамку рисует базовый QLabel
    QLabel::paintEvent(event);
}
